# QNLP from first principles: unified engineering specification

Quantum natural language processing (DisCoCat / pregroup grammar -> parameterised circuits, lambeq-style) for a
C / C++ / Mojo implementation on CUDA GPUs and Qualcomm silicon (Hexagon HVX/HMX/VTCM, Adreno) under the design
goal "NLP on quantum theory with almost zero memory". Everything below is pasteable into source comments:
plain-ASCII maths, C-like pseudocode, no Python. Facts that could not be verified are marked UNVERIFIED with
the best available estimate. Equation labels are (S.n); section references are written "Sec. S.n".

---------------------------------------------------------------------------------------------------------------

## 0. Conventions (fixed once; every other section obeys them)

C1  Bit order (little-endian). A register of W qubits is one array psi[0 .. 2^W-1] of complex numbers. Qubit j
    is bit j of the array index: i = sum_j b_j * 2^j, bit_j(i) = (i >> j) & 1. Qubit 0 is the least
    significant bit and the fastest-varying index. As a ket, |b_{W-1} ... b_1 b_0>.

C2  Kronecker order. When (and only when) a state is written as a Kronecker product, the LEFT factor is the
    HIGH bits: (A (x) B)[rA*dimB + rB][cA*dimB + cB] = A[rA][cA] * B[rB][cB]. We avoid Kronecker notation for
    words and sentences and use explicit index maps instead (C7), because the linguistic reading order
    (left-to-right type string) is the OPPOSITE of Kronecker order.

C3  Storage. Complex arrays are SPLIT PLANAR: re[0..2^W-1] and im[0..2^W-1], two contiguous arrays of the same
    scalar type (fp32, fp16 or int16 Q1.15); a real-valued ansatz uses re[] only. All index formulas apply to
    both planes identically. (On CUDA, one thread owning whole amplitudes may use interleaved float2; nothing
    else changes.) Matrices are row-major: M[r][c] at offset r*ncols + c; r = output row, c = input column.
    Column vectors; an operator acts from the left: psi' = U psi.

C4  Order of application. OPERATOR PRODUCTS are read right-to-left (rightmost applied first): U = G_m ... G_2 G_1
    means G_1 is applied first. GATE LISTS are read left-to-right (first listed = first applied). Every ansatz
    below is given as a gate list.

C5  Rotations. R_P(theta) = exp(-i * theta * P / 2), theta in radians. With c = cos(theta/2), s = sin(theta/2):

      Rx(theta) = [[ c,    -i s ], [ -i s,  c   ]]                                          (0.1)
      Ry(theta) = [[ c,    -s   ], [  s,    c   ]]                                          (0.2)
      Rz(theta) = [[ c - i s, 0 ], [ 0, c + i s ]] = diag(e^{-i theta/2}, e^{+i theta/2})   (0.3)
      H         = (1/sqrt2) [[ 1, 1 ], [ 1, -1 ]]                                           (0.4)

    Rz differs from diag(1, e^{i theta}) by a global phase; CRz does NOT (Sec. 0, C6). The HALF-ANGLE
    phi = theta/2 is what the kernel needs (cos phi, sin phi). Angles are STORED as 16-bit binary half-angle
    turns: k in uint16, phi = 2*pi*k/65536, i.e. theta = 4*pi*k/65536. Quantisation |d_phi| <= pi/65536 =
    4.8e-5 rad; since ||dR/dphi|| = 1 the state error per gate is <= 4.8e-5. Wrap-around under addition is
    exact. This encoding is 4*pi-periodic in theta, which is required for controlled rotations (Sec. 5.6).
    On CUDA: sincospif(k * (1.0f/32768.0f), &s, &c) gives (sin phi, cos phi) with no range reduction.
    lambeq/DisCoPy expose a phase parameter in half-turns: theta = 2*pi*phase (UNVERIFIED against the exact
    library version; verify one exported Rz and one CRz numerically before importing weights).

C6  Two-qubit gates. For qubits lo < hi (qubit NUMBERS), the four coupled amplitudes share all bits except lo and
    hi; the LOCAL index is m = bit_lo(i) + 2*bit_hi(i) in {0,1,2,3}; a dense 4x4 is U4[m_out][m_in], row-major.
    Controlled gates are DEFINED by bit tests, never by a permuted matrix. CNOT(c->t) means control c, target t.

      CNOT(c->t): for every i with bit_c(i)=1 and bit_t(i)=0: swap psi[i], psi[i | (1<<t)]                (0.5)
      CZ:         psi[i] *= -1 when bit_c(i)=bit_t(i)=1                                                    (0.6)
      CRz(theta)(c->t): for bit_c(i)=1: psi[i] *= (bit_t(i) ? e^{+i theta/2} : e^{-i theta/2})            (0.7)
      CRx(theta)(c->t): apply Rx(theta) to the pair (i, i|(1<<t)) for every i with bit_c(i)=1, bit_t(i)=0   (0.8)

    In the local order m = bit_lo + 2 bit_hi: CRz with control hi, target lo = diag(1, 1, e^{-i th/2},
    e^{+i th/2}); with control lo, target hi = diag(1, e^{-i th/2}, 1, e^{+i th/2}). CNOT(hi->lo) swaps
    m=2,3; CNOT(lo->hi) swaps m=1,3. The convention diag(1,1,1,e^{i theta}) for CRz is a DIFFERENT gate
    (it differs by an Rz on the control); whichever produced an angle table must be used.

C7  Words, wires, fields. A word w has pregroup type t_1 t_2 ... t_m (left to right). Type factor i occupies
    q(t_i) consecutive qubits starting at the local offset

      fo_w[i] = sum_{i' < i} q(t_{i'}),  q(n)=q(n^l)=q(n^r)=q_n,  q(s)=q(s^l)=q(s^r)=q_s,  Q_w = sum_i q(t_i)  (0.9)

    so the FIRST (leftmost) factor is the LOWEST bits. The word's state is the 2^{Q_w} array phi_w, and the
    tensor view is phi_w[v_1 + 2^{fo_w[2]} v_2 + ... ] for factor values v_i. For a transitive verb n^r s n^l:
    V[a, s, b] = phi[a + 2^{q_n} s + 2^{q_n+q_s} b], a = n^r (low), s (middle), b = n^l (high). In the
    monolithic sentence register, word w starts at off_w = sum_{w' < w} Q_{w'} and simple-type position p
    (global, left to right) has field start(p) = off_{word(p)} + fo_{word(p)}[factor(p)], width(p) = q(b_p).

C8  Cups. The contraction t^l t -> 1 or t t^r -> 1 between two q-qubit wires is the UNNORMALISED Bell effect
    <cup_q| = sum_{k=0}^{2^q-1} <k| (x) <k| = sqrt(2^q) <Phi+_q|, paired COMPONENT-WISE: bit r of wire A with
    bit r of wire B, r = 0..q-1. (The nested/planar pairing a_{q-1-r} <-> b_r, which lambeq's DisCoPy cups
    build for q >= 2, is a different but equally valid model: UNVERIFIED which one lambeq uses; identical for
    q = 1. If nested pairing is wanted, replace k_c by bitreverse_q(k_c) in the B field.) On a simulator a cup
    is a plain index contraction (Sec. 1.3); in a circuit it is CNOT(a->b), H(a), postselect |00> per qubit
    pair, which equals the contraction times 2^{-q/2} per cup; all such factors cancel in normalisation.

C9  Symbols. Ng = number of gates in a circuit. D(theta) = postselection denominator (Sec. 5.1) -- never a gate
    count. u = unit roundoff (fp32 2^-24 = 6.0e-8; fp16 2^-11 = 4.9e-4; bf16 2^-8; Q1.15 store rounding
    2^-16 absolute). p_surv = postselection survival probability. grad = gradient. W = qubits of the register
    under discussion; N = 2^W. "FMA-class op" = one real multiply or one fused multiply-add; a complex MAC is 4.

C10 Ansatz identifiers (stored in the model header, Sec. 6.3) and angle layouts theta_w[0..P_w-1]:
      ansatz_id 0  IQP, no closing H layer (DisCoPy IQPansatz to the author's recollection, UNVERIFIED)
      ansatz_id 1  IQP with a closing H layer on every qubit
      ansatz_id 2  Sim14,  ansatz_id 3  Sim15,  ansatz_id 4  Ry-only (real)
    Q = 1 words (all ansatze except 4): gate list Rx(t0), Rz(t1), Rx(t2); P_w = 3; theta_w = [t0, t1, t2].
    Q = 1 word under ansatz 4: Ry(t0)|0>, P_w = 1.
    IQP layer l (l = 0..L-1) on Q >= 2 qubits: H on qubits 0..Q-1, then CRz(theta_w[l*(Q-1)+j])(j -> j+1) for
      j = 0..Q-2.  ansatz 1 appends H on every qubit after the last layer.  P_w = L (Q-1).
    Sim15 layer: Ry(theta_w[l*2Q + j]) on all j; CNOT(j -> (j-1) mod Q) for j = 0..Q-1; Ry(theta_w[l*2Q+Q+j])
      on all j; CNOT(j -> (j+1) mod Q) for j = 0..Q-1.  P_w = 2 Q L.  Real-valued for every Q >= 2.
    Sim14 layer: as Sim15 with CRx(u) in place of CNOT: angles theta_w[l*4Q + {0, Q, 2Q, 3Q} + j] = t_j, u_j,
      t'_j, u'_j in application order.  P_w = 4 Q L.
    Ry-only (ansatz 4): Ry(theta_w[j]) on each qubit j, then a fixed CNOT ladder CNOT(0->1), CNOT(1->2), ...;
      P_w = Q; real.  (Ring directions/orders for Sim14/15 are UNVERIFIED against lambeq; any fixed order is
      a valid ansatz; state the chosen one in the header.)
    Consequence of ansatz 0 with L = 1: every amplitude of the word has modulus exactly 2^{-Q/2}; only phases
    carry information. Use L >= 2 or ansatz 1 if magnitude variation is wanted.

C11 Transpose of a word unitary. U_w^T = G_1^T G_2^T ... G_m^T: run the GATE LIST IN REVERSE ORDER with each
    gate transposed. Rx, Rz, H, CNOT, CZ, CRz, CRx are symmetric matrices (unchanged); Ry(theta)^T = Ry(-theta).
    So: IQP / Rx-Rz-Rx: reversed order, same angles. Sim14/Sim15/Ry-only: reversed order, Ry angles negated,
    CNOT/CRx unchanged. Only for the product ansatz Ry-only WITHOUT the CNOT ladder does this equal U_w(-theta)
    in the original order.

C12 fp16 facts (IEEE binary16): 1/5/10 bits, 11-bit significand, u = 2^-11 = 4.88e-4, max 65504, smallest
    normal 2^-14 = 6.1e-5, subnormals to 2^-24 = 6e-8. fp32: u = 2^-24. A normalised W-qubit state has
    typical |psi[i]| = 2^{-W/2} (normal in fp16 to W = 28), but |psi[i]|^2 ~ 2^{-W} is fp16-subnormal from
    W = 15 and a term below 2^-11 of the running sum is absorbed: probabilities are NEVER accumulated in fp16.

C13 Fixed gates, generators and dense 4x4 forms (the reference for the fused 4x4 path of Sec. 2.2 and for T2).

      I = [[1, 0], [0, 1]]    X = [[0, 1], [1, 0]]    Y = [[0, -i], [i, 0]]    Z = [[1, 0], [0, -1]]         (0.10)

    Rx = exp(-i theta X/2), Ry = exp(-i theta Y/2), Rz = exp(-i theta Z/2): generators G = X, Y, Z (G^2 = I).
    CRz(theta)(c->t) = exp(-i theta G/2) with G = |1><1|_c (x) Z_t;  CRx(theta)(c->t): G = |1><1|_c (x) X_t.
    These G have eigenvalues {0, +1, -1} (G^2 = |1><1|_c (x) I != I), so an expectation f(theta) contains
    cos(theta/2) and sin(theta/2) terms and the two-term shift rule fails for them (Sec. 5.2). Inside U^T (C11)
    every gate keeps the same generator; only the Ry angle changes sign. Dense 4x4 in the local order
    m = bit_lo(i) + 2 bit_hi(i) (C6), rows = m_out, cols = m_in, c = cos(theta/2), s = sin(theta/2):

      CNOT(hi->lo) = [[1,0,0,0],[0,1,0,0],[0,0,0,1],[0,0,1,0]]   swaps m = 2,3;  = |0><0|_hi (x) I + |1><1|_hi (x) X  (C2)
      CNOT(lo->hi) = [[1,0,0,0],[0,0,0,1],[0,0,1,0],[0,1,0,0]]   swaps m = 1,3;  = I (x) |0><0|_lo + X (x) |1><1|_lo
      CZ           = diag(1, 1, 1, -1)
      CRz(hi->lo)  = diag(1, 1, c - i s, c + i s)                CRz(lo->hi) = diag(1, c - i s, 1, c + i s)
      CRx(hi->lo)  = [[1,0,0,0],[0,1,0,0],[0,0,c,-i s],[0,0,-i s,c]]   (Rx on the m = 2,3 block)
      CRx(lo->hi)  = [[1,0,0,0],[0,c,0,-i s],[0,0,1,0],[0,-i s,0,c]]   (Rx on the m = 1,3 block)               (0.11)
      lift_lo(G) = I (x) G = [[G, 0], [0, G]]  (block-diagonal in m):
                   entry [m_out][m_in] = G[bit0(m_out)][bit0(m_in)] * (bit1(m_out) == bit1(m_in))
      lift_hi(G) = G (x) I:  entry [m_out][m_in] = G[bit1(m_out)][bit1(m_in)] * (bit0(m_out) == bit0(m_in))   (0.12)

    Dense W-qubit reference matrix (for T2, no Kronecker bookkeeping needed): with mask = (1 << lo) | (1 << hi),
    M[i_out][i_in] = U4[m(i_out)][m(i_in)] if (i_out & ~mask) == (i_in & ~mask), else 0; for a 1q gate on k:
    M[i_out][i_in] = U[bit_k(i_out)][bit_k(i_in)] if (i_out & ~(1 << k)) == (i_in & ~(1 << k)), else 0. This IS
    the C2 Kronecker product I (x) .. (x) U (x) .. (x) I written index-wise; compare apply_1q / apply_2q against
    M psi on random psi for every k and every lo < hi at W = 6 (T2), including both control orientations.

---------------------------------------------------------------------------------------------------------------

## 1. The model: grammar -> circuit -> readout

### 1.1 Pregroup grammar and parser

Basic types n (noun phrase), s (sentence). Encode a simple type as (b, z), b in {N, S}, z in Z: z = 0 is t,
z = -1 is t^l, z = +1 is t^r, z = -2 is t^ll, z = +2 is t^rr. Adjoints add or subtract 1 from z. The only
rewrite needed to decide grammaticality (Lambek's switching lemma; expansions are never needed) is

    (b, z) (b, z+1) -> 1     for two ADJACENT simple types, left neighbour has the smaller z.           (1.1)

Lexicon (fixed table, one type string per lexical class; a word's type is looked up, never inferred):

    noun / proper noun        n              [(N,0)]
    adjective, determiner     n n^l          [(N,0),(N,-1)]
    intransitive verb         n^r s          [(N,+1),(S,0)]
    transitive verb           n^r s n^l      [(N,+1),(S,0),(N,-1)]
    adverb (post-verbal)      s^r s          [(S,+1),(S,0)]

Shift-reduce parser. Input: words; output: cups (pairs of global simple-type positions) and the position of the
surviving s, or FAIL. Because the cups are a non-crossing matching, a stack suffices:

    parse(words):
        types = []                       // flat list of records {b, z, word_id, factor_id}
        for w in words: for each factor i of lexicon[w]: types.append({b_i, z_i, w, i})
        stack = []; cups = []
        for p in 0 .. len(types)-1:
            if stack nonempty and types[top(stack)].b == types[p].b and types[top(stack)].z == types[p].z - 1:
                cups.append((pop(stack), p))
            else: push(stack, p)
        if len(stack) == 1 and types[stack[0]].b == S and types[stack[0]].z == 0: return (cups, stack[0])
        return FAIL

Greedy contraction is exact for every shape in the table below (each simple type has at most one admissible
partner). For a wider lexicon (relative pronouns n^r n s^l n, double adjoints) replace the inner test by a
depth-first search over {contract, shift} bounded by the sentence length, or keep the shape table as ground
truth. Positions are counted over simple types, left to right.

    shape            type string                          cups                          s position
    N IV             n . n^r s                            (0,1)                         2
    N TV N           n . n^r s n^l . n                    (0,1) (3,4)                   2
    ADJ N TV N       n n^l . n . n^r s n^l . n            (1,2) (0,3) (5,6)             4
    N TV ADJ N       n . n^r s n^l . n n^l . n            (0,1) (3,4) (5,6)             2
    ADJ N TV ADJ N   n n^l . n . n^r s n^l . n n^l . n    (1,2) (0,3) (5,6) (7,8)       4
    N IV ADV         n . n^r s . s^r s                    (0,1) (2,3)                   4

(ADJ N TV N: the adjective's n^l at 1 contracts with the noun at 2; then the adjective's n at 0 is on top and
contracts with the verb's n^r at 3 -- nested, non-crossing.)

Position -> qubit field (C7): start(p) = off_{word_id[p]} + fo_{word_id[p]}[factor_id[p]], width(p) = q(b_p).
For a cup (p, p') both widths are equal by construction.

### 1.2 The DisCoCat functor

F(n) = C^{2^{q_n}}, F(s) = C^{2^{q_s}}, F(t^l) = F(t^r) = F(t) (finite dimension: dual identified with the
space), F(t_1 ... t_m) = tensor product (with the index map of C7). A word's meaning is the pure state

    phi_w = U_w(theta_w) |0>^{(x) Q_w}     (2^{Q_w} complex amplitudes, generated on the fly from P_w angles)   (1.2)

Total monolithic sentence width W = sum_w Q_w. The sentence state (unnormalised) is obtained by applying every
cup to the product of all word states:

    sigma[s] = sum_{k_1=0}^{2^{q_1}-1} ... sum_{k_C=0}^{2^{q_C}-1}  Psi[ idx(k_1..k_C, s) ],
    idx(k_1..k_C, s) = sum_c k_c * (2^{start(A_c)} + 2^{start(B_c)}) + s * 2^{start(S)},                        (1.3)
    Psi[i] = prod_w phi_w[ (i >> off_w) & (2^{Q_w} - 1) ]

where (A_c, B_c) are the two type positions of cup c, S the surviving s position, and every wire is either
cupped or S (else parse failed). The fields are disjoint and k_c < 2^{width}, so (1.3) is exactly the
component-wise pairing of C8. There are 2^{q_s} outputs, each a gather-and-sum over 2^{sum_c q_c} addresses.

Normalisation and physical postselection probability:

    Z = sum_s |sigma[s]|^2,   sigma_hat = sigma / sqrt(Z),   P_post(Bell circuit) = Z / 2^{sum_c q_c}       (1.4)

Z can be 0 for a degenerate parameter set: compute it in fp32; if Z < Z_min (1e-12 fp32) the sentence is
undefined (return uniform probabilities, zero gradient). E[Z] = 1 for Haar-random words; the trained model
alone drives Z small. For N TV N (C7 index map):

    sigma[s] = sum_a sum_b n1[a] * V[a, s, b] * n2[b],   V[a,s,b] = phi_V[a + 2^{q_n} s + 2^{q_n+q_s} b]     (1.5)

with NO complex conjugation anywhere (the cup is a transpose, not an inner product).

### 1.3 Contraction order: the fused tensordot (the recommended kernel)

Never form the product state Psi. Sweep words left to right, keeping one open tensor T whose fields are the
still-open type positions, and contract each incoming word against T over the cups that join them, with the
multiply-accumulate FUSED into the loop (never T (x) phi_w first):

    // T: open tensor with fields FT[0..nT-1] = {pos, width}, stored little-endian in list order:
    //    addr_T(v_0..v_{nT-1}) = sum_i v_i << (width_0 + ... + width_{i-1})
    // phi: word state with factor fields FW[i] = {pos, width}, shift = fo_w[i]  (C7)
    // cups joining T and w: pairs (iT, iW); every other field of T goes to restT, of w to restW
    // scatter(x, fields): deposit consecutive bit groups of x (widths of the fields, in order) at the fields' shifts
    for rT in 0 .. 2^{sum widths(restT)} - 1:
      for rW in 0 .. 2^{sum widths(restW)} - 1:
        acc = 0
        for k in 0 .. 2^{sum widths(cup fields)} - 1:
          aT = scatter(rT, restT) | scatter(k, cupT)        // k's r-th bit group lands in cupT[r] and cupW[r]
          aW = scatter(rW, restW) | scatter(k, cupW)
          acc += T[aT] * phi[aW]                            // complex MAC, fp32 accumulate
        R[(rW << widthsum(restT)) | rT] = acc              // new open tensor: restT fields low, restW high
    T = R;  FT = restT ++ restW

Peak per step = |T| + |phi_w| + |R| (or |T| + |phi_w| if R overwrites a dead buffer). The rank of T never
exceeds the largest word. Fold sequences (q_n = q_s = 1 amplitude counts in brackets):

    N TV N:      T=n1 [2];  R[s,b] = sum_a n1[a] V[a,s,b] [4];  sigma[s] = sum_b R[s,b] n2[b] [2].  Peak 2+8+4 = 14 (+2 for n2 = 16)
    N IV:        sigma[s] = sum_a n[a] IV[a,s], IV[a,s] = phi[a + 2^{q_n} s]
    ADJ N TV N:  n'[a] = sum_b ADJ[a,b] n[b], ADJ[a,b] = phi[a + 2^{q_n} b] (a = n factor, b = n^l);  then as N TV N
    N IV ADV:    R[b] = sum_a n[a] IV[a,b];  sigma[s] = sum_b R[b] ADV[b,s], ADV[b,s] = phi[b + 2^{q_s} s]

General peak: 2^{2 q_n + q_s} + 2^{q_n} + 2^{q_n + q_s} (+ 2^{q_n}): (1,1) -> 14-16 amplitudes; (2,1) -> 44-48.
This is exact, cheaper than any circuit, and W-independent in the sense that the monolithic 2^W is never formed.

In-place slot assignment (the "10 in place" figure of Sec. 7). Rule: the output entry for free-index tuple f
overwrites the contracted-index-0 slot of the INCOMING word tensor at f, after both (all 2^q) contracted-index
entries at f have been read into registers; since every output at f depends only on the entries at f, there is
no ordering hazard, in any loop order. At (1,1) N TV N:
    R[s,b] = n1[0] V[0,s,b] + n1[1] V[1,s,b]   is written into phi_V[0 + 2s + 4b]  (the a = 0 slot; read both
             a-terms of that (s,b), then write);
    n2 is then loaded into n1's two slots (n1 is dead after the first step);
    sigma[s] = R[s,0] n2[0] + R[s,1] n2[1]     is written into phi_V[2s]  (the b = 0 slot of R, i.e. a = 0, b = 0).
Working set = 8 (verb) + 2 (noun) = 10 complex; at (2,1): 32 + 4 = 36 (R[s,b] into phi_V[0 + 4s + 8b], 8 entries;
sigma[s] into phi_V[4s]). Register cost of the copies is nil: they are renamings after unrolling.

### 1.4 Bend-the-wire rewrite (circuit form, needed for hardware and for adjoint training)

Since <cup|(phi_n (x) I) = sum_a n[a] <a| (x) I, a noun followed by a cup equals the EFFECT <n^T| = <0| U_n^T
(transpose, no conjugation) on the partner wire: apply U_n^T (C11) to the partner wire and postselect |0..0>.
A word can be an effect only if it is a bare U_w|0> whose every wire cups to a STATE word's wire; a contracted
intermediate (n' = ADJ.n, or n.V) is not unitarily preparable and cannot be bent. Minimum circuit width:

    shape            state words                     effect words (U^T on the partner wires)      W_hw
    N IV             IV (q_n + q_s)                  N on IV.n^r                                  q_n + q_s
    N TV N           TV (2 q_n + q_s)                N1 on TV.n^r, N2 on TV.n^l                   2 q_n + q_s
    ADJ N TV N       N (q_n), TV                     ADJ on (TV.n^r, N.n) as a 2 q_n-qubit effect   3 q_n + q_s
    N TV ADJ N       TV, N2 (q_n)                    N1 on TV.n^r, ADJ on (TV.n^l, N2.n)            3 q_n + q_s
    ADJ N TV ADJ N   N1 (q_n), TV, N2 (q_n)          ADJ1 on (TV.n^r, N1.n), ADJ2 on (TV.n^l, N2.n)  4 q_n + q_s
    N IV ADV         N (q_n), ADV (2 q_s)            IV on (N.n, ADV.s^r)                          q_n + 2 q_s

An effect word's factor i is applied (via U_w^T) to the qubits of the state wire it cups to; the postselected
set K is all those qubits; S is the surviving s wire (belonging to a state word). No Bell measurement is ever
needed for the table shapes; the sentence amplitude on S with K = 0 equals sigma[s] EXACTLY (no 2^{-q/2}
factor), and p_surv = Z. State-word qubits are numbered left to right over state words only. Example N TV N,
(1,1): TV on qubits 0 (n^r), 1 (s), 2 (n^l); U_N1^T on qubit 0; U_N2^T on qubit 2; K = {0, 2}, S = {1}.

Bent-wire wire maps for every table shape at (q_n, q_s) = (1, 1). State-word qubits are numbered left to right
over the state words in SENTENCE order (first state word = lowest qubits), factors within a word by C7. An
effect word's local qubit j (its j-th type factor, C7) is placed on the physical qubit wire_map[j] = the
state-word qubit that the factor's type position cups to (Sec. 1.1 cups). Every gate of the effect word's
REVERSED gate list (C11) has each qubit index j replaced by wire_map[j]: G1(j) -> G1(wire_map[j]),
CNOT(j -> j') -> CNOT(wire_map[j] -> wire_map[j']), CRz(j -> j') likewise; a mapped 2q gate whose qubits are
non-adjacent or in swapped order is simply the (lo, hi) = (min, max) instance of (2.3) with the control
ORIENTATION preserved (C6 is defined by bit tests, C13 gives both orientations), nothing else changes.
K = the union of all wire_map entries, M_post = OR of (1 << q) over K, S = the surviving s qubit (a state-word
qubit), b(c) = c << S.

    shape            state words -> qubits                     effect words: wire_map            K             S  M_post   b(c)    W
    N IV             IV: q0 = n^r, q1 = s                      N^T: [0]                          {0}           1  0b001    c << 1  2
    N TV N           TV: q0 = n^r, q1 = s, q2 = n^l            N1^T: [0];   N2^T: [2]            {0, 2}        1  0b101    c << 1  3
    ADJ N TV N       N: q0;  TV: q1 = n^r, q2 = s, q3 = n^l    ADJ^T: [1, 0];  N2^T: [3]         {0, 1, 3}     2  0b1011   c << 2  4
    N TV ADJ N       TV: q0 = n^r, q1 = s, q2 = n^l;  N2: q3   N1^T: [0];   ADJ^T: [2, 3]        {0, 2, 3}     1  0b1101   c << 1  4
    ADJ N TV ADJ N   N1: q0;  TV: q1, q2, q3;  N2: q4          ADJ1^T: [1, 0];  ADJ2^T: [3, 4]   {0, 1, 3, 4}  2  0b11011  c << 2  5
    N IV ADV         N: q0;  ADV: q1 = s^r, q2 = s             IV^T: [0, 1]                      {0, 1}        2  0b011    c << 2  3

Derivations (type positions from the Sec. 1.1 table). ADJ N TV N: ADJ factor 0 (n, pos 0) cups pos 3 = TV.n^r
= q1; factor 1 (n^l, pos 1) cups pos 2 = N.n = q0 -> wire_map [1, 0]; N2 (pos 6) cups pos 5 = TV.n^l = q3.
N TV ADJ N: N1 (pos 0) cups pos 1 = TV.n^r = q0; ADJ factor 0 (n, pos 4) cups pos 3 = TV.n^l = q2; factor 1
(n^l, pos 5) cups pos 6 = N2.n = q3 -> [2, 3]. ADJ N TV ADJ N: ADJ1 -> [1, 0] as above; ADJ2 factor 0 (pos 6)
cups pos 5 = TV.n^l = q3, factor 1 (pos 7) cups pos 8 = N2.n = q4 -> [3, 4]. N IV ADV: IV factor 0 (n^r, pos 1)
cups pos 0 = N.n = q0; factor 1 (s, pos 2) cups pos 3 = ADV.s^r = q1 -> [0, 1]. The state/effect assignment is
forced in every row: a word all of whose wires cup to state-word wires is an effect; a word with the surviving
s, or with a wire cupped to an effect word, is a state. Example of a mapped list: ADJ under IQP L = 1 has gate
list H(0) H(1) CRz(t)(0 -> 1); ADJ^T with wire_map [1, 0] is applied as CRz(t)(1 -> 0), H(0), H(1) (reversed,
then mapped; CRz and H are symmetric). Any permutation of the physical qubits is a valid relabelling (Sec. 3.1
and 3.2 use it to put the verb's qubits on in-thread / whole-vector bits); the table is the canonical numbering
used by the PROGRAM and by the tests (T5, T6).

General (q_n, q_s): replace every single qubit above by a field of width q(b) in the same order (state word
fields at off_w + fo_w[i], C7); a wire_map entry becomes a field START, and bit r of the effect word's factor
field (its local qubit fo_w[j] + r) maps to physical qubit start + r (component-wise pairing, C8; for nested
pairing use start + q - 1 - r). K = all bits of all mapped fields; S = the q_s bits of the surviving s field;
b(c) = scatter(c, S field) = c << start(S); M_post = OR of (1 << q) over K. Postselection of K on a live state
uses (2.5) for qubit pairs and (2.5') for a single qubit, or -- for readout only -- no compaction at all
(Sec. 4.5: N_c = |psi[b(c)]|^2, D = sum over (i & M_post) == 0).

### 1.5 Readout

    p[k] = |sigma[k]|^2 / Z,  k = 0 .. 2^{q_s}-1                                                            (1.6)
    K classes, K <= 2^{q_s}:  p_K[k] = (|sigma[k]|^2 + eps) / (sum_{j<K} |sigma[j]|^2 + K eps),  Loss = -ln p_K[y]   (1.7)

eps = 1e-9 with fp32 accumulation (always form (1.7) in fp32; in fp16 eps is absorbed unless eps > u * sum).
Meaning similarity of two normalised sentence states: Fid = |sum_k conj(a[k]) b[k]|^2, unnormalised
Fid = |<sigma_1|sigma_2>|^2 / (Z_1 Z_2). A swap test is never worth simulating. Sentence embedding = 2^{q_s}
complex numbers. If cup qubits are traced out instead (mixed state) rho = Tr_K |psi><psi|:
rho[c][c'] = sum_{kk} psi[b(c,kk)] conj(psi[b(c',kk)]) with b(c,kk) = b(c) | scatter(kk, K fields); for
q_s = 1 the Uhlmann fidelity is F = Tr(rho_1 rho_2) + 2 sqrt(det rho_1 det rho_2).

### 1.6 Quantum-theory-inspired classical alternatives

Vector model: store v_w (d_w complex, d_w = 2^{Q_w} or any mixed-radix product d_n^{#n} d_s^{#s}, with
V[a,s,b] = v[a + d_n s + d_n d_s b]); composition is (1.3)/(1.5) verbatim; zero gates, 2^{Q_w} storage per word.
Density-matrix model: rho_w Hermitian PSD d x d; composition contracts each cup on row AND column index:
sigma_rho[s,s'] = sum_{a,a',b,b'} rho_n1[a,a'] rho_V[(a,s,b),(a',s',b')] rho_n2[b,b'], p[k] = sigma_rho[k,k] /
Tr sigma_rho; for pure words it reduces to |sigma><sigma|. Storage d^2 reals (real upper + imaginary strict upper
triangle). Transitive verb at (2,1): d = 32: 32 complex (256 B) vector; 1024 fp16 (2 KiB) density matrix; 4 L
uint16 angles (8 L bytes) as IQP. Hardware note: the circuit model is SIMT/HVX work only; the vector and
density models at d = 32 map onto a 32x32 fp16 HMX tile / tensor-core tile when 32 sentences are batched.

---------------------------------------------------------------------------------------------------------------

## 2. The simulation kernel

### 2.1 Single-qubit butterfly (in place, no matrix materialised)

A gate U on qubit k couples exactly the index pairs differing in bit k:

    for every i0 with bit_k(i0) = 0, i1 = i0 | (1 << k):
        psi'[i0] = u00 psi[i0] + u01 psi[i1];   psi'[i1] = u10 psi[i0] + u11 psi[i1]                      (2.1)
    pair enumeration j in [0, 2^{W-1}):  i0(j,k) = ((j >> k) << (k+1)) | (j & ((1 << k) - 1))              (2.2)

(2.2) is a strictly increasing bijection onto {i : bit_k(i) = 0} (verified exhaustively at W = 6).
Worked example (hand-checkable): W = 4, k = 1: j = 0..7 -> i0 = 0, 1, 4, 5, 8, 9, 12, 13 (every value has
bit 1 = 0), i1 = i0 | 2 = 2, 3, 6, 7, 10, 11, 14, 15. Single case j = 5 = 0b101: (5 >> 1) << 2 = 0b1000 = 8,
5 & 1 = 1, i0 = 9 = 0b1001, i1 = 11 = 0b1011.

    /* cplx = {re, im}; cmul(a,b) = {a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re}; cadd = componentwise.
       fp16 pipelines widen to fp32 on load, compute, narrow on store. */
    void apply_1q(cplx* psi, int W, int k, const cplx U[2][2]) {
        uint64_t low = (1ull << k) - 1;
        for (uint64_t j = 0; j < (1ull << (W-1)); ++j) {
            uint64_t i0 = ((j >> k) << (k+1)) | (j & low), i1 = i0 | (1ull << k);
            cplx a = psi[i0], b = psi[i1];
            psi[i0] = cadd(cmul(U[0][0], a), cmul(U[0][1], b));
            psi[i1] = cadd(cmul(U[1][0], a), cmul(U[1][1], b));
        }
    }

Cost: 16 FMA-class per pair, F_1q(W) = 2^{W+3}; traffic 2 S(W) bytes if streamed (intensity 1 flop/byte -->
memory-bound by 1-2 orders of magnitude if the state is off-chip; hence Sec. 3). Free cases: diagonal U (Rz):
psi[i] *= d_{bit_k(i)}, 2^{W+2} ops, no exchange. X: relabel with a flip mask (Sec. 2.4). H: adds only,
scale folded into the next gate.

### 2.2 Two-qubit gates and fusion

lo < hi, quad enumeration j in [0, 2^{W-2}), lower insertion first:

    t   = ((j >> lo) << (lo+1)) | (j & ((1<<lo)-1));  i00 = ((t >> hi) << (hi+1)) | (t & ((1<<hi)-1));
    idx[m] = i00 | (bit0(m) << lo) | (bit1(m) << hi),  m = 0..3;   v' = U4 v                             (2.3)
    compact(i) (inverse of i00, for bit_lo(i)=bit_hi(i)=0):
      compact(i) = ((i >> (hi+1)) << (hi-1)) | (((i >> (lo+1)) & ((1 << (hi-lo-1)) - 1)) << lo) | (i & ((1<<lo)-1))   (2.4)

i00(j) >= j, strictly increasing (verified W = 6, all lo < hi). Worked example: W = 4, lo = 1, hi = 3, j = 2 =
0b10: t = ((2 >> 1) << 2) | (2 & 1) = 4; i00 = ((4 >> 3) << 4) | (4 & 7) = 4 (= 0b0100: bits 1 and 3 clear);
idx[0..3] = 4, 6, 12, 14 (= i00, i00 | 2, i00 | 8, i00 | 10). Inverse: compact(4) = ((4 >> 4) << 2) |
(((4 >> 2) & 1) << 1) | (4 & 1) = 0 | 2 | 0 = 2 = j. Dense 4x4: 2^{W+4} FMA-class. Diagonal 2q gates
(CZ, CRz) are one streaming pass: psi[i] *= D[bit_lo(i) + 2 bit_hi(i)]. CNOT: permutation, zero flops.

Fusion: consecutive gates on the same qubit compose as U_total = U_n ... U_1 (C4), 32 FMA-class per product --
negligible against 2^{W+3}. A 1q gate G adjacent to a 2q gate U4 on (lo, hi) lifts as lift_lo(G) = I (x) G,
lift_hi(G) = G (x) I (C2: hi is the high local bit; explicit entries in (0.12), 2q matrices in (0.11)). Fuse
greedily per qubit set; diagonal gates commute with
everything on other qubits and can be pulled to the nearest butterfly on their qubit. Fused matrices are
generated in registers from angles and never stored. Fusion removes lane EXCHANGES, not just arithmetic
(Sec. 3.1), and on the fixed-point path it also halves the rounding error (Sec. 4.3).

Generation cost: one sincos per angle = 20-40 FMA-class (polynomial) against a butterfly of 2^{W+3}:
W=4: 16-31 %; W=6: 4-8 %; W=8: 1-2 %; W=10: 0.25-0.5 %; W=12: <= 0.12 %. At W <= 6 (the (1,1) regime)
generation is a first-order cost: generate once per fused word gate and share across sentences that use the
word, or use the exported (cos, sin) pairs (Sec. 6.3).

### 2.3 Postselection and cups on a live statevector

Projecting qubits lo < hi onto <00| and dropping them:

    psi'[j] = psi[i00(j)],  j in [0, 2^{W-2});   p_surv = sum_j |psi'[j]|^2;   W -= 2                     (2.5)

In-place safe in ASCENDING j because i00(j) >= j (equality for j < 2^{lo}: the read precedes the write within
the iteration). Dropped quarters are never touched: read 2^{W-2}, write 2^{W-2}, p_surv accumulated in the same
pass in fp32 (widen BEFORE squaring). The Bell cup (a state-to-state cup, needed only when two state words are
cupped directly, e.g. the monolithic circuit) is the DIRECT GATHER, not (2.5) alone:

    <00| H_a CNOT(a->b) = (<00| + <11|)/sqrt2, so
    z = psi[i00(j)] + psi[i11(j)],  i11 = i00 | (1<<a) | (1<<b);  psi'[j] = z;  p += |z|^2;  (drop 1/sqrt2)   (2.6)

Also in-place safe (i11(j) > i00(j) >= j). Applying (2.5) without (2.6)'s sum silently discards every |11>
component (e.g. psi = |11>: Bell effect gives 1/sqrt2, bare projection gives 0). In the bent-wire form
(Sec. 1.4) there are no state-state cups: only (2.5) is used, after U^T.

Single-qubit postselection (odd |K|, e.g. ADJ N TV N with K = {0, 1, 3}, or N IV with K = {0}): project qubit k
onto <0| and drop it:

    psi'[j] = psi[i0(j, k)],  j in [0, 2^{W-1});   p_surv = sum_j |psi'[j]|^2;   W -= 1                     (2.5')

In-place safe in ascending j since i0(j, k) >= j (2.2). A set K of any size is removed by applying (2.5)/(2.5')
in DESCENDING qubit order, so that the numbers of the qubits still to be removed are unchanged (removing a
higher qubit never renumbers a lower one). For READOUT no compaction is needed at all: N_c = |psi[b(c)]|^2 and
D = sum over i with (i & M_post) == 0 (Sec. 4.5) read the uncompacted vector directly; compaction is only for a
circuit that continues after the projection.

Worked example of (2.5) (the Sec. 8 circuit): W = 3, K = {0, 2}, lo = 0, hi = 2: j = 0: t = 0, i00 = 0;
j = 1: t = ((1 >> 0) << 1) | 0 = 2, i00 = ((2 >> 2) << 3) | (2 & 3) = 2. So psi'[0] = psi[0], psi'[1] = psi[2],
p_surv = |psi[0]|^2 + |psi[2]|^2 -- exactly the b(c) = c << 1 of Sec. 8 (N_0 = |psi[0]|^2, N_1 = |psi[2]|^2).

Renormalisation: fold 1/sqrt(p_surv) into the next gate's matrix or, better, never renormalise (Sec. 4.5):
every output is a ratio. Carry log(p_total) = sum_c log p_c in fp32 if the sentence norm is wanted.

Early cups (live-state algorithm for the monolithic form). A cup on (a, b) may be applied as soon as no
remaining gate touches a or b; since word circuits act only on their own wires, that is "both words are
online". Words not yet online need not exist in the array. Bring word w online by Kronecker-in with the word in
the HIGH bits, in place in DESCENDING destination order:

    for d = 2^{W_live + Q_w} - 1 down to 0:  psi[d] = phi_w[d >> W_live] * psi[d & (2^{W_live} - 1)];  W_live += Q_w   (2.7)

(source index <= d, read before overwritten). Schedule for N TV N: online N1 (q_n) -> online V (peak
3 q_n + q_s; phi_V also live: + 2^{2 q_n + q_s}) -> Bell cups N1.n -- V.n^r (q_n + q_s) -> online N2
(2 q_n + q_s) -> cups V.n^l -- N2.n (q_s) -> readout. Peak 2^{3 q_n + q_s} + 2^{2 q_n + q_s} = 16 + 8 = 24
amplitudes at (1,1), 128 + 32 = 160 at (2,1), versus 2^{4 q_n + q_s} = 32 / 512 naive and 14-16 / 44-48
for the fused tensordot (Sec. 1.3), which never forms the Kronecker product. Peak over a general sweep =
2^{max over steps (open wires before the step + Q_w of the incoming word)}; the tensordot peak is
2^{max(Q_w, open wires after the step)} plus buffers, smaller by 2^{k} where k is the qubits cupped in that step.

### 2.4 Flip-mask relabelling (X gates only)

Maintain m (W bits); the physical index of logical i is i ^ m; X on qubit k is m ^= (1 << k). Every rule in
Sec. 2 is written in logical indices: replace psi[i] by psi[i ^ m], and in Sec. 3.1's coefficient selector use
b = bit_k(t ^ m). Before a postselect (2.5), Kronecker-in (2.7) or readout, either materialise the mask (one
swap pass) or fold it into the gather (psi'[j] = psi[i00(j) ^ m]) and reset m = 0. CNOT is NOT a flip mask
(it is GF(2)-linear i -> i ^ (bit_c(i) << t), which breaks the compaction proof); implement CNOT as the
permutation (0.5).

### 2.5 Tensor-network contraction as a matmul

Contracting A (rank ca qubits, contracted k qubits in its LOW bits) with B (rank cb, contracted k in its HIGH
bits):

    out[(ia << fb) | ib] = sum_{ci < 2^k} A[(ia << k) | ci] * B[(ci << fb) | ib],  fa = ca-k, fb = cb-k     (2.8)

which is the row-major product of A_mat (2^{fa} x 2^k) by B_mat (2^k x 2^{fb}); cost 2^{ca+cb-k} complex MAC;
result rank fa + fb with ia in the high bits. If the contracted qubits are elsewhere, permute bits once
(2^{rank} gathers) -- free at program-compile time since word tensors are generated by us. When 2^k >= 32 this
is HMX / tensor-core shaped (Sec. 3.3). The fused tensordot of Sec. 1.3 is the general-field version of (2.8).

### 2.6 Program representation and batching

Bucket sentences by (shape, W): all sentences in a bucket share one PROGRAM and differ only in angle values, so a
warp / vector never diverges and every qubit index, offset and loop bound is a COMPILE-TIME constant (template
parameter / Mojo comptime) -- required for register residency (Sec. 3.1). Program opcodes:
ONLINE(word_pos, Q_w) [(2.7)]; G1(k, slot, kind); G2(lo, hi, slot, kind) [slot relative to the current word's
angle row]; TRANSPOSED(word_pos, wire_map) [U^T of an effect word, C11]; CUP_BELL(a, b) [(2.6)];
POST00(a, b) [(2.5)]; POST0(k) [(2.5')]; CONTRACT(fields) [Sec. 1.3]; READOUT(S fields). Fusion is applied at
program-compile time.

G1 / G2 field definitions. kind in {RX, RY, RZ, H, X, DIAG1, DENSE2} for G1 and {CNOT, CZ, CRZ, CRX, DIAG2,
DENSE4} for G2; for CNOT/CRZ/CRX the opcode carries (c, t) (control, target) and the kernel derives (lo, hi) =
(min, max) and the orientation per C6/C13 when it forms the matrix. slot = index of the FIRST angle of the gate
in the current word's angle row theta_w (C10), -1 for angle-free gates (H, X, CNOT, CZ); n_angles(kind)
consecutive slots are consumed: RX/RY/RZ/CRZ/CRX 1; DENSE2 fused from Rx-Rz-Rx 3; DIAG1/DIAG2 fused from n
diagonal gates n; DENSE4 fused from one 1q gate (lifted per (0.12)) and one 2q gate 1 + 1. A fused gate whose
angles are not consecutive stores a short list of (slot, count) pairs (at most 4 per opcode in this version).
The fused matrix is generated in registers from those slots per C4 / C2 / C13 (Sec. 2.2) and never stored.
Word context: ONLINE / TRANSPOSED set the "current word" whose param_offset (Sec. 5.5, 6.3) resolves slots into
the global angle table; a TRANSPOSED context also negates the applied Ry angle (C11) and sets sign_k = -1 for
RY gates in the adjoint (Sec. 5.3). Every field is a compile-time constant of the bucket's program.
Kernel inputs: program[Ng] (constant/uniform memory), word_ids[B][n_words] (2-4 B each), angle table
theta[V][slots] (THE model, Sec. 7), outputs[B][2^{q_s} + 1] (probabilities + Z or log p).

---------------------------------------------------------------------------------------------------------------

## 3. Hardware mapping

### 3.1 SIMT / CUDA

What the hardware is: an SM issues warps of 32 threads (converged threads issue together; since Volta threads
have independent PCs, so lockstep is guaranteed only by keeping the program uniform per block and using the
*_sync shuffle forms with mask 0xffffffff). Each thread has private 32-bit registers: 255 max per thread,
65536 per SM (256 KB); at 1024 threads/block the cap is 64/thread (compile with __launch_bounds__(1024,1)).
__shfl_xor_sync(0xffffffff, x, m) moves one 32-bit value (or a __half2) from lane (lane ^ m) within a warp,
32 lanes per clock per SM. fp32 FMA: 64/clk/SM (cc 7.0, 8.0), 128/clk/SM (8.6, 8.9, 9.0, 10.x). Shared memory:
128 B/clk/SM (32 banks x 4 B); static __shared__ <= 48 KB per block; larger via extern dynamic shared memory
plus cudaFuncSetAttribute(kernel, cudaFuncAttributeMaxDynamicSharedMemorySize, bytes). Per-block maxima:
cc 7.0: 96 KB; 7.5: 64 KB; 8.0: 163 KB; 8.6/8.9/12.0: 99 KB; 9.0/10.0: 227 KB (per-SM figure minus 1 KB).
Resident threads/SM: 2048 (8.0, 9.0, 10.x), 1536 (8.6, 8.9, 12.0). HBM: 1.5-2 TB/s (A100), 3.35 TB/s
(H100 SXM), ~8 TB/s (B200); aggregate shared-memory bandwidth ~10x HBM; register file ~10x again.

REGISTER RESIDENCY IS A PROPERTY OF INDEXING, NOT SIZE. nvcc keeps a per-thread array in registers only if every
index is a compile-time constant after unrolling; a runtime qubit index, W, shape or word offset demotes the
whole array to local memory (L1/L2-backed loads/stores -- exactly the traffic being avoided). Therefore: one
kernel instantiation per (shape, q_n, q_s, ansatz, L); all gate loops fully unrolled; gates on "local" qubits
implemented as a switch over the compile-time qubit positions. Verify with -Xptxas -v (or the Mojo equivalent):
0 bytes stack/local memory, 0 spill loads/stores.

Layout A -- THREAD-PER-SENTENCE (recommended for the fused tensordot, Sec. 1.3). The whole per-sentence working
set is private to one thread: (1,1): 16 complex fp32 = 32 registers + ~20 temporaries; (2,1): 48 complex =
96 registers + temporaries, ~110-128 in total (fp16 storage buys nothing on CUDA -- precision, not the register
file, is the constraint there). Block size follows from the register count: at (2,1) launch with <= 512
threads/block (65536 / 128; the 1024-thread cap of 64 registers/thread is NOT enough), at 256 threads/block the
cap is 255; at (1,1) (~52 registers) 1024 threads/block is fine. Verify with -Xptxas -v: 0 spill stores.
No shuffles, no barriers, no shared memory. A warp = 32 sentences of one bucket.
Angle gather: word_id -> row of the angle table (L2/L1-resident, 100 KB class); per-sentence DRAM traffic =
word ids in, 2^{q_s} + 1 floats out.

Layout B -- WARP-PER-SENTENCE, two amplitudes per thread (monolithic or bent-wire circuits, W <= 6; one warp
holds exactly W = 6, smaller W share a warp, W = 7..10 use the register-blocked form below). Thread t of 2^{W-1} threads owns
A0 = psi[t], A1 = psi[t | (1 << (W-1))]. For a gate on qubit k < W-1 both of t's amplitudes have bit_k = bit_k(t)
and both partners live on thread t ^ (1 << k):

    b = bit_k(t);  A' = u[b][b] * A + u[b][1-b] * A_partner        (both threads do distinct work)         (3.1)

k = W-1: local, full butterfly in registers. k < 5: partner via 4 shuffles (re/im of A0, A1; 2 if fp16x2
packed as two amplitudes' re or two sentences' re -- never (re, im) of one amplitude in one half2). Dense 2q
gate on (lo, hi), lo < hi < W-1: m = bit_lo(t) + 2 bit_hi(t); v[c] = amplitude of thread
t ^ ((bit0(c^m)) << lo) ^ ((bit1(c^m)) << hi), c = 0..3 (c = m is own); A' = sum_c U4[m][c] v[c]: 3 partner
fetches, 64 FMA-class per thread. Diagonal 2q gates: per-thread multiply, no exchange. CNOT: exchange on the
target bit restricted to control-set threads. For W <= 6 the whole state fits one warp (64 amplitudes): no
shared memory, no barriers; at (1,1) the bent-wire N TV N (W = 3, 8 amplitudes) is 4 threads -- 8 sentences
per warp, 256 per block. Shuffle cost: a gate on a lane-bit qubit costs 4 SHFL warp-cycles against 16*32/128 = 4
FMA warp-cycles: balanced on 128-FMA parts, FMA-bound on A100; place the most-rotated qubits (the verb's) at
k = W-1 / in-thread bits by compile-time relabelling.

Layout B, W < 6 (several sentences per warp). 2^{6-W} sentences per warp; sentence s occupies lanes
[s 2^{W-1}, (s+1) 2^{W-1}); lane t holds A0 = psi_s[t'], A1 = psi_s[t' | 2^{W-1}] with t' = t & (2^{W-1} - 1)
and s = t >> (W-1). __shfl_xor_sync(0xffffffff, x, 1 << k) with k < W-1 never leaves the sentence's lane group
(the xor touches a bit below the group width), so (3.1) is unchanged. Reductions (D, N_c) are SEGMENTED: an
xor-shuffle tree with masks 1, 2, ..., 2^{W-2} only (W-1 levels); afterwards every lane of a group holds its
group's sum and lane t' = 0 writes the sentence's output. At (1,1) N TV N (W = 3): 4 lanes per sentence, 8
sentences per warp, reduction masks 1 and 2; K = {0, 2}: qubit 0 is a lane bit (lanes with bit_0(t') = 1 are
masked out of D), qubit 2 = W-1 is the in-thread bit (A0 survives, A1 is dropped): N_0 = |A0|^2 on lane
t' = 0, N_1 = |A0|^2 on lane t' = 2, i.e. b(c) = c << 1 as in Sec. 8.

Layout B-blocked -- one warp per sentence, W = 7..10 (W = 11 marginal). Lane t (0..31) holds R = 2^{W-5}
amplitudes A[r] = psi[(r << 5) | t], r = 0..R-1: index bits 0..4 = lane id, index bits 5..W-1 = register
number r. Registers: 2R fp32 = 2^{W-4} (W = 7: 8; 8: 16; 9: 32; 10: 64; 11: 128 -> block <= 256 threads for
the 255 cap and no room for the adjoint's three vectors). Every r is a compile-time constant after unrolling,
so A[] stays in registers (the residency rule above).
    qubit k < 5 (a LANE bit, below the lane-width threshold): for every r: P = __shfl_xor_sync(0xffffffff,
        A[r], 1 << k) (re and im separately: 2R shuffles per gate); b = bit_k(t); A[r] = u[b][b] A[r] +
        u[b][1-b] P -- (3.1) applied R times, 8R FMA-class per lane.
    qubit k >= 5 (a REGISTER bit, above the threshold): in-register butterfly (2.1) between A[r0] and
        A[r0 | (1 << (k-5))] for r0 enumerated by (2.2) with (W-5, k-5): zero exchange, zero barriers,
        8R FMA-class per lane.
    dense 2q (lo, hi): both >= 5: in-register quad (2.3) over r with (W-5, lo-5, hi-5), no exchange.
        lo < 5 <= hi: for each register pair (r0, r1 = r0 | (1 << (hi-5))) fetch the partner lane's A[r0],
        A[r1] (4 shuffles); own m = bit_lo(t) + 2 bit_{hi-5}(r); v[m'] = {own A[r0], own A[r1], partner A[r0],
        partner A[r1]} at their m'; A'[r] = sum_{m'} U4[m(r)][m'] v[m'] for r = r0, r1 (32 FMA-class per pair).
        both < 5: the 3-fetch rule of Layout B applied to every r (6R shuffles).
    diagonal 2q / CRz / CZ: per-register multiply with the coefficient selected by (bit_lo, bit_hi) of
        ((r << 5) | t); CNOT: register renaming when the target bit is >= 5, shuffle on control lanes otherwise.
    postselect (2.5)/(2.5'): a K qubit k >= 5 is a pure register renaming (keep the registers whose bit
        (k-5) of r is 0), no data movement, W -= 1 per qubit; a K qubit k < 5 makes lanes with bit_k(t) = 1
        DEAD: do NOT compact across lanes (shuffles cost more than they save at W <= 10) -- keep them idle and
        mask them out of the D / N_c reduction with the lane predicate (t & M_post_lanes) == 0.
    reduction: per-lane partial sums over the live registers (exact fp32 adds), then a 5-level xor-shuffle
        tree (one sentence per warp: no segmenting).
    relabel at compile time so that the verb's (most-rotated) qubits and, where possible, the K qubits are
        register bits >= 5; then the only shuffles left are the effect words' rotations on lane bits.
The training layout (Sec. 6.1) with three vectors A, B, C fits this form up to W = 8 (3 x 16 = 48 registers).

Layout C -- BLOCK-PER-SENTENCE (W in [7, 11] one pair per thread, where the register-blocked Layout B is not used; larger W with each thread looping over pairs
p = t, t + T, ... so W is bounded by shared memory, not by the 1024-thread cap). Qubits 5 <= k < log2(T)
exchange through shared memory: sm[t] = A0; sm[t+T] = A1; __syncthreads(); read partner; ping-pong two buffers
for one barrier per gate. Cost of an exchange gate: 2 S(W) / 128 B cycles per SM plus barrier skew; W = 14
fp32: 2048 cycles vs the gate's 2^17/128 = 1024 FMA cycles on 128-FMA parts (2x smem-bound; 1x with fp16x2
exchange buffers). Relabel so hot qubits avoid the 5..log2(T)-1 band. Postselection (2.5)/(2.6): all threads
keep executing the program with an 'active' flag; barriers are executed unconditionally; compaction is one
smem round trip (write at compact(i), barrier, re-read the two new owned amplitudes) plus a block reduction for
p_surv (warp shuffle tree -> 32 partials in smem -> final warp). Bank conflicts: pairs at stride 2^k for k < 5
fold 2:1 onto the 32 banks -- prefer register-blocking (each thread holds 2^m amplitudes and applies all gates on
those m qubits barrier-free).

    W   amplitudes  fp32 bytes  layout          threads   regs/thread(est.)  smem exchange     notes
    3   8           64 B        A (or B: 4 thr) 1 / 4     ~50 / ~28          0                 (1,1) bent-wire N TV N
    5   32          256 B       B, 16 thr       16        ~28                0                 (1,1) monolithic N TV N
    6   64          512 B       B, one warp     32        ~28                0
    8   256         2 KB        B-blocked R=8   32        ~40 (16 state)     0                 or C, 4 warps, 128 thr, 2 KB smem (k=5,6)
    10  1024        8 KB        B-blocked R=32  32        ~90 (64 state)     0                 or C, 512 thr, 8 KB smem, 2-4 blocks/SM
    12  4096        32 KB       C               1024      ~32 (<= 64 cap)    32 KB / 16 KB f16 2 blocks/SM on 2048-thread SMs
    14  16384       128 KB      C, 16 amps/thr  1024      ~56 (<= 64 cap!)   128 KB fp32       only 163/227 KB parts; else 64 KB fp16x2

(Register counts are compiler-dependent targets to verify with ptxas, not facts.) Global memory is touched only
for the program (uniform, once per block), word ids and angles (once), and 2^{q_s} + 1 outputs.

Training layout (Sec. 5.3 adjoint): three state vectors A, B, C of 2^W complex fp32 = 24 * 2^W B: W = 10: 24 KB;
W = 11: 48 KB = exactly the static limit with zero headroom -- declare them as dynamic shared memory with the
opt-in attribute and budget ~2 KB extra (reduction scratch, w_c); the two-vector streaming variant needs
16 * 2^W B (32 KB at W = 11). Gate list in __constant__ memory, not shared. Ng + 2 Ng block barriers dominate
at W <= 11; at (1,1) the bent-wire circuits are W = 2..5 and Layout B (one warp, no barriers) is the right one.

### 3.2 Hexagon HVX (1024-bit vector unit) and VTCM

What the hardware is: 32 vector registers V0..V31 of 1024 bits = 128 B (4096 B total per HVX context), 4
predicate registers Q0..Q3 (128 bits, one bit per byte; they cannot hold data). Lanes: 128 x int8, 64 x int16 /
fp16, 32 x int32 / fp32. Up to 4 instructions per packet. Resources (LLVM Hexagon scheduling model,
UNVERIFIED against the V68/V73 HVX PRM 80-N2040-47/-54): 2 multiply units (a widening double-vector multiply
occupies both), 1 permute (xlane) unit, 1 shift unit, 1 load slot, 1 store slot. VTCM: on-NPU scratch,
~1 vector load + 1 store per cycle; 8 MB on v73/v75/v79-class mobile parts (Snapdragon 8 Gen 2 onward) and
per Cloud AI 100 core (confirmed); 256 KB - 4 MB on v66-v69 parts (UNVERIFIED per SoC). VTCM is shared by all
DSP clients and must be reserved (HAP compute-resource manager); assume only a fraction is available; contents
are not retained across DSP power collapse (UNVERIFIED). Parallelism = HVX contexts (2 on <= SM8150, ~4 on
recent parts, UNVERIFIED), one sentence stream per context.

Floating point: NO vector floating point before v68 (Snapdragon 888 class); v6x parts are integer-only. On v68+
the default codegen is Qualcomm's non-IEEE qf16/qf32 (about one bit less precision than IEEE, u ~ 2^-10;
values must be converted before permutes, stores or compares). IEEE-result forms (Vd.hf = vmpy(hf,hf),
Vx.hf += vmpy(hf,hf), Vdd.sf = vmpy(hf,hf), Vxx.sf += vmpy(hf,hf), Vd.sf = vadd(sf,sf)) require the
'hvx-ieee-fp' feature; the error model of Sec. 4 assumes that path. No vector fp divide/sqrt/rsqrt through
v81 (do 1/sqrt(p) on the scalar core). No scalar-operand fp multiply before v79: each of the 8 real gate
constants of a 2x2 gate must be splatted into a vector register (8 registers; a fused 4x4 needs 32 = the whole
file). fp16 rounding mode and subnormal handling (flush-to-zero likely) are UNVERIFIED: measure fl(a*b) against
an fp32 reference once on target. Widening fp32 accumulate (Vxx.sf += vmpy(hf,hf)) produces a register PAIR
and occupies both multiply units: 2x multiply cycles and 2x temporaries versus pure hf.

Integer path (every HVX generation, v60+): int16 x int16 -> int32 widening multiplies WITH A SCALAR OPERAND:
Vdd.w = vmpy(Vu.h, Rt.h), Vxx.w += vmpy(Vu.h, Rt.h), Vd.w = vdmpy(Vu.h, Rt.h):sat (2-term complex-multiply
primitive) -- gate constants cost ZERO vector registers. The Q1.15 store (Sec. 4.3) is ONE instruction:
Vd.h = vasr(Vu.w, Vv.w, Rt8):rnd:sat (Rt8 = 14) packs two int32 vectors into one int16 vector with
round-half-up and saturation. Caveat: the widening multiply's pair lane order (even/odd de-interleave vs
low/high split) and which half of Rt.h multiplies which lanes are UNVERIFIED -- splat the constant into both
halves (Rt = (c << 16) | (c & 0xFFFF)) and unit-test the round trip with a ramp vector; a lane-order mistake
silently permutes amplitudes, an O(1) error invisible to every bound in Sec. 4. The non-saturating accumulate
is safe by the bound of Sec. 4.3. Q1.31: Vd.w = vmpye(Vu.w, Vv.uh) then Vx.w += vmpyo(Vu.w, Vv.h):<<1:rnd:sat:
shift gives the Q1.31 x Q1.31 -> Q1.31 high product in 2 ops (vector x vector only, ~1 LSB error per product,
not exact); no 64-bit lanes exist.

Layout: split fp16/int16 planes (C3). Amplitude i lives at vector v = i >> 6, lane l = i & 63 of each plane;
a W-qubit state is 2^{W-5} vectors. W = 10 fp16 = 4096 B = the ENTIRE register file (no room for products).
Realistic register residency: W <= 8 comfortable (8 state vectors), W = 9 tight (16 state vectors + 8 splats
+ operands = 32-36), W >= 10 streams from VTCM. Throughput floor per 1q gate: cycles >= 2^{W-7} *
ceil(16 / n_mpy) with n_mpy = 2 (UNVERIFIED) = 2^{W-4} (W = 14: 1024 cycles; W = 10: 64), set by the multiply
units; loads + stores per gate are 2^{W-5} each, which fit under that floor with the load/store slots half idle.
CONSEQUENCE: keeping the state in VTCM rather than registers costs nothing in throughput once latency is
hidden; there is no "W = 10 cliff". Registers are for constants, permute controls and software-pipelining
temporaries; VTCM holds state, angle table, permute-control constants. Off-chip (DDR) traffic stays zero for
weights and state after load (instruction fetch, token ids in, probabilities out excepted).

32-bit lanes (IEEE fp32 'sf' state on v68+ with hvx-ieee-fp, or int32 Q1.31): amplitude i lives at vector
v = i >> 5, lane l = i & 31 of each plane; a W-qubit state is 2^{W-4} vectors (both planes); W = 9 fp32 = 32
vectors = 4096 B = the ENTIRE register file, so residency is W <= 7 comfortable (16 vectors), W = 8 tight,
W >= 9 streams from VTCM. The lane-width threshold moves from 6 to 5: gates on qubit k >= 5 are whole-vector
pairs ((2.2) with W-5, k-5); k < 5 are in-vector partners l ^ (1 << k) via vdelta with FIVE control vectors
(640 B in VTCM; k = 0 is a 32-bit word swap, k = 4 a 64-byte rotate, UNVERIFIED as single opcodes) and 4
coefficient vectors per k (2.5 KB) or 4 vmux per gate. Every index formula of this section is otherwise
unchanged with 6 -> 5. The sentence-per-lane (1,1) tensordot holds 32 sentences per vector on 32-bit lanes.
Widening accumulate Vxx.sf += vmpy(hf, hf) produces a register PAIR whose lane assignment (even/odd
de-interleave vs low/high halves of the 64 hf lanes) is UNVERIFIED: treat it exactly like the int16 widening
pair (splat both halves, ramp-vector round-trip unit test, T9) and derive the 32-bit layout above from the
measured order before writing any index formula that crosses the hf/sf boundary.

Gate on qubit k >= 6: bit k of i is bit (k-6) of the vector number, so pairs are whole vectors (v0 from (2.2)
with W-6, k-6): 16 mul + 12 add = 28 vector ops per pair (16 with vmpy_acc), pure lane arithmetic, no permute.
Gate on qubit k < 6: partners are lanes l and l ^ (1 << k) of the SAME vector. Per plane vector: one xor-lane
permute (vdelta with a constant 128-B control vector, one per k = 0..5, 768 B in VTCM; control-byte
conventions UNVERIFIED -- validate with a known permutation at bring-up; k = 0 is a halfword swap and k = 5 a
64-byte vror, UNVERIFIED as opcodes) and lane-dependent coefficients Cs[l] = bit_k(l) ? u11 : u00,
Co[l] = bit_k(l) ? u10 : u01 (4 vmux ops per gate against a lane-id compare, or 4 precomputed vectors per k
in VTCM = 3 KB), then x' = Cs*x + Co*partner: 14 ops (8 fused). At W = 10: 16 vectors x (2 permutes + 14) + 4
= 260 ops of which 32 permutes (single permute unit: ~32 cycles, in parallel with 128 multiply cycles, not on
the critical path) vs 224 for k >= 6. Register-lean variant: vdeal two consecutive state vectors jointly into
(all bit_k = 0 lanes ; all bit_k = 1 lanes) as two FULL registers, apply the whole-register butterfly with
uniform (scalar) coefficients, vshuff back; same 2 permutes per plane vector. Relabelling: choose the logical->
physical qubit map at compile time so the most-rotated qubits sit at k >= 6; a full 64 x 2^{W-6} transpose
(swap low 6 index bits with high bits) costs 192-384 permutes each way at W = 12 (pair-producing shuffles
UNVERIFIED as 1 or 2 ops) versus 128 extra permutes per low-qubit gate: break-even ~6 consecutive low-qubit
gates round trip, ~3 if the transposed layout is kept and all later index maps use the permuted bit order.

Per-(q_n, q_s) layout for the fused tensordot (Sec. 1.3):
(1,1) SENTENCE-PER-LANE: 64 sentences per vector; the in-place working set (verb 8 + noun 2 = 10 complex) is 20
vectors re/im; on the Q1.15 path constants are scalar operands, leaving 12 registers for products/temporaries:
fits, zero permutes, every gate is a lane-parallel scalar-coefficient FMA over 64 sentences, and the sincos
generation (Sec. 4.4) is amortised over 64 angles at once (~10 ops per gate per 64 sentences). On the fp16 path
pre-v79 the 8 coefficient splats per gate make it spill -- use Q1.15 there.
(2,1) AMPLITUDE-PER-LANE: one sentence per (re, im) vector pair for the 32-amplitude verb (or two sentences per
pair, 32 lanes each); gates on in-vector bits use vdelta as above; reductions (Z, p_surv) by 5 vror + vadd
steps (there is no cross-lane horizontal add; fp32 vadd only on v68+, int32 always). Anything beyond 32 vectors
lives in VTCM (still on-chip).

### 3.3 Matrix units (HMX; Cloud AI 100 tensor unit; tensor cores)

A butterfly is U(2x2) times X(2 x 2^{W-1}): contraction depth K = 2, so a K = 32 tile (HMX fp16 32x32, 2 KiB)
or a K = 16 mma.sync tile is 1/16 - 1/8 utilised and operands must be gathered into tile rows per gate -- a
permutation pass per gate. Matrix units cannot help butterflies. They can do: (i) fused m-qubit block unitaries
with 2^m >= 32: relabel the m targets to the LOW bits, view psi as Psi_mat[hi][lo] (2^{W-m} x 2^m, row-major)
and compute Psi_mat * U_m^T (or targets in the HIGH bits and U_m * Psi_mat[lo][hi]); a 32x32 complex fp16 U_m
= 4 KB (8 KB real-embedded via z -> [[a,-b],[b,a]]) must be MATERIALISED TRANSIENTLY in VTCM (HMX consumes
VTCM-resident tiles in a fixed, partly reverse-engineered layout) -- never persisted, never off-chip -- and its
fp16 quantisation ||E||_F <= u16 sqrt(32) = 2.8e-3 dominates the store rounding (c ~ 6 in Sec. 4), so it is an
accuracy win only if it replaces >= 6 separate gates; (ii) the vector/density models and any tensordot with
2^k >= 32 (Sec. 2.5), e.g. 32 sentences x (32x32 verb matrix) at (2,1). HMX facts: exists from v68; fp16 tile
32x32, int8/int16/int4 tiles (fp16 possibly v73+, UNVERIFIED); accumulation width UNVERIFIED; its ISA is
publicly documented only from V81 (80-N2040-62) -- on v68-v79 it is reachable only via QNN/HTP or undocumented
encodings, so HMX use is conditional on the adapter actually exposing it. Cloud AI 100: 16 Hexagon-derived
cores per SoC, each 8 MiB VTCM + 1 MB L2 (144 MB on-die), vector unit 512 int8 / 256 fp16 MAC/clk (assumed
HVX-1024 with 32 registers, UNVERIFIED), tensor unit 8192 int8 / 4096 fp16 MAC/clk, LPDDR4x 136 GB/s; fp16 and
int8 confirmed, int16/fp32 UNVERIFIED; the public SDK is a graph compiler -- the adapter is the only kernel
route. Per-core VTCM holds W <= 20 fp16 statevectors, so on this part the design is purely multiply-unit bound.
Tensor cores: products fp16 x fp16 are exact in fp32 (22 <= 24 bits); accumulation reportedly aligns to the
largest exponent and truncates (UNVERIFIED per architecture) -- never use them for the probability sums.

### 3.4 Adreno (OpenCL / Vulkan compute)

Wave (sub-group) 64 fibers on 6xx/7xx (128 mode on some parts, UNVERIFIED); CL_DEVICE_LOCAL_MEM_SIZE = 32 KB per
work-group (confirmed on Adreno 730), CL_DEVICE_MAX_WORK_GROUP_SIZE = 1024, cl_khr_fp16 native (~2x fp32 rate,
UNVERIFIED), cl_khr_subgroups + cl_qcom_subgroup_shuffle (shuffle_up/down/rotate/xor incl. half),
cl_qcom_reqd_sub_group_size; register file ~64 KB per shader processor, ~16 fp32 registers per fiber at full
occupancy (derived, UNVERIFIED); spills go to SYSTEM memory (worst case); local memory is not necessarily
faster than the L2 path (Qualcomm guide). Mapping: Layout A (fiber-per-sentence) for the fused tensordot at
(1,1) (16 complex fp16 = 16 packed half2 registers -- at the occupancy limit; reduce occupancy or use (1,1)
in-place 10 complex); Layout B with sub-group shuffles for W <= 7 (64 fibers x 2 amplitudes); local-memory
exchange up to W <= 12 fp32 / 13 fp16 with zero slack at the top. Replace the constant 5 (log2 warp) by
log2(sub-group size) in every shuffle/local boundary. Forward-only: the 48 KB adjoint layout does not fit.
The monolithic (2,1) ADJ N TV N state (8192 amplitudes, 32 KiB fp16) fills local memory exactly and the
doubly-modified shape fits nowhere -- the fused tensordot is MANDATORY on Adreno, not merely recommended.

---------------------------------------------------------------------------------------------------------------

## 4. Number formats and error bounds

### 4.1 Why nothing can grow

sum_i |psi_i|^2 = 1 and every gate is unitary, so |psi_i| <= 1, |Re|, |Im| <= 1 forever; an error vector e is
propagated with ||U e|| = ||e|| -- errors ADD linearly, never amplify (unlike a neural net). For one butterfly
row psi'_a = u_a0 psi_0 + u_a1 psi_1 with |u_a0|^2 + |u_a1|^2 = 1, the real part is a 4-term dot product

    Re psi'_a = x.y,  x = (Re u_a0, -Im u_a0, Re u_a1, -Im u_a1),  y = (Re psi_0, Im psi_0, Re psi_1, Im psi_1)
    Im psi'_a = x'.y, x' = (Im u_a0, Re u_a0, Im u_a1, Re u_a1),   ||x|| = ||x'|| = 1, ||y|| <= 1              (4.1)

and Cauchy-Schwarz on ANY subset of terms bounds every partial sum by 1 in exact arithmetic (the loose
magnitude-only bounds are sqrt2 for 2x2 and 2 for 4x4: 1 guard bit covers sqrt2, the 4x4 loose bound 2 needs
2 guard bits, which the int32 Q3.29 accumulator below has). Fixed point therefore needs no scale factors.

### 4.2 Floating point

Higham's dot-product bound |fl(x.y) - x.y| <= gamma_n ||x|| ||y||, gamma_n ~ n u, with n = 4 gives per real
component 4u ||psi_pair||, per pair 8u ||psi_pair||, and summing squares over all pairs

    ||psi'_computed - U psi|| <= 8u ||psi||   (arithmetic in the storage format) + sqrt2 u (gate entries rounded)
    per-gate constant c:  pure-fp16 arithmetic 2x2: c = 10;  4x4: c = 25 (2.5x a 2x2);                     (4.2)
                          gate entries generated in fp16 arithmetic: c ~ 24 (never do it);
                          fp16 STORAGE with fp32 compute and fp32 gate entries: c = 1 (only the store rounds);
                          fp16 storage, fp32 compute, fp16 gate-entry vectors (HVX): c = 1 + sqrt2 = 2.4.
    ||hat_psi_Ng - psi_Ng|| <= Ng c u (1 + O(Ng c u))  worst;  typical (random walk) ~ 2u sqrt(Ng) for pure fp16,
                          ~0.58 u sqrt(Ng) for store-only rounding (RMS of a uniform [-u,u] error).            (4.3)
    probability of any projector:  |delta p| <= 2 ||delta psi|| + ||delta psi||^2.                          (4.4)

The typical 2u figure is an estimate (4 roundings of RMS u/sqrt3 on running values ~0.5 ||psi_pair|| give
~1.15u, gate rounding ~1.15u in quadrature, ~1.6u rounded up). fp16 -> fp32 conversion is exact and fp16 x fp16
is exact in fp32, so "fp16 storage / fp32 compute" costs nothing on CUDA cores (fp32 FMA rate == unpacked fp16
rate) and forfeits only the packed-fp16 rate on HVX/Adreno, where a 2x2 kernel is permute/bandwidth-bound anyway.
bf16 (u = 2^-8) trades 3 significand bits for exponent range the problem never needs: unusable for amplitudes.
Angles: ||dR/dtheta|| = 1/2; fp16 angles near pi have ulp 2^-9 = 2e-3 rad (state error ~1e-3 per gate, as large
as the whole Q1.15 budget for 100 gates) -- store 16-bit half-angle turns (C5), error 4.8e-5 per gate.
Underflow: the model assumes gradual underflow; with flush-to-zero (UNVERIFIED on HVX/Adreno; fast-math on GPUs)
each flushed product loses up to 2^-14, i.e. up to sqrt(2N) 4 2^-14 = 2^{W/2 - 11.5} per gate when gate entries
are below 2^{W/2 - 14} (near-identity rotations at init): 2.8e-3 at W = 10, comparable to c u. Disable FTZ for
the state kernel or use Q1.15, which has no such mode.

### 4.3 Fixed point: Q1.15 (int16) and Q1.31 (int32)

Q1.15: value n/2^15, n in [-32768, 32767], range [-1, 1 - 2^-15]; +1.0 is NOT representable: initialise
|0..0> as re[0] = 0x7FFF (= 1 - 2^-15, relative error 3e-5 that cancels in every ratio) and SATURATE on every
store (a rotation of quantised entries has row norm up to 1 + 4.3e-5). Scheme for a 2x2 gate:

    state:   Re, Im in Q1.15 int16, planar.
    gate:    Re u, Im u in Q2.14 int16 (range [-2, 2-2^-14]) so that exactly +-1 (identity/Rz(0), X, Z, CZ, CNOT
             entries) is representable; entry rounding <= 2^-15.
    product: int16 x int16 -> int32, Q3.29, EXACT.  Sum of 4 (8 for 4x4) products: int32, exact;
             loose bound sqrt2 2^29 (2x2) / 2^30 (4x4) < 2^31 (a Q1.15 gate would put the 4x4 bound at exactly 2^31).
    store:   n' = sat_int16((acc + 2^13) >> 14)   -- arithmetic shift, round-to-nearest, saturate. NEVER truncate:
             truncation has bias -2^-16 per component per gate accumulating coherently in the worst case.       (4.5)
    Per-gate error: ||e_round|| <= sqrt(2N) 2^-16 = 2^{W/2 - 15.5} (worst), ~0.82 2^{W/2-16} (typical);
             gate quantisation ||E||_F <= sqrt8 2^-15 = 8.6e-5 (2x2), 1.7e-4 (4x4; note the SAME single store
             rounding, so a fused 4x4 counts as ~1 gate, not 2.5: fusion HALVES fixed-point error).
    g_Q15(W) = 2^{W/2-15.5} + 8.6e-5:  W=8: 4.3e-4   W=10: 7.8e-4   W=12: 1.5e-3   W=16: 5.6e-3            (4.6)
    g_typ(W) = sqrt((0.82 2^{W/2-16})^2 + (6e-5)^2):  W=8: 2.1e-4  W=10: 4.05e-4  W=12: 8.0e-4  W=16: 3.2e-3
    Norm drift per gate: rotations <= 4.3e-5, general 2x2 <= 8.6e-5, 4x4 <= 1.7e-4 (norm, not norm-squared).

Error is ABSOLUTE and W-dependent (scales with sqrt N), whereas fp16's c u ||psi|| is W-independent:
component-wise the two are equal at W = 10; in 2-norm Q1.15 beats pure fp16 up to W ~ 16 (worst) / ~12
(typical), and fp16-storage/fp32-compute (c = 1, g = 4.9e-4) equals Q1.15 at W = 9 and wins for W >= 10.
Q1.15's genuine advantages: the only vector format on HVX v6x; zero-register gate constants on all HVX;
EXACT int32 probability sums (Sec. 4.5).
Q1.31: state int32 Q1.31, |0..0> = 0x7FFFFFFF, gate Q2.30 (rounding 2^-31), product int32 x int32 -> int64
Q3.61 exact, int64 accumulate (loose bound 2^62 < 2^63), store sat_int32((acc + 2^29) >> 30);
g_Q31(W) = 2^{W/2-31.5} + 1.3e-9 (W=16: 8.5e-8; W=24: 1.4e-6). Needs 32x32->64 and 64-bit accumulation:
CUDA cores and the Hexagon scalar unit; on HVX only via the vmpye/vmpyo pair (~1 LSB per product) -- prefer
fp32 there if IEEE fp is available.

### 4.4 Generating cos/sin from 16-bit half-angle turns (zero table memory)

k as uint16 (two's-complement wrap exact). Octant reduction: oct = k >> 13; r = k & 0x1FFF; if (oct & 1)
r = 0x2000 - r; x = r * (pi/4) / 8192 in [0, pi/4]; (s, c) = (sin x, cos x); then
oct 0: (s, c); 1: (c, s); 2: (c, -s); 3: (s, -c); 4: (-s, -c); 5: (-c, -s); 6: (-c, s); 7: (-s, c) gives
(sin phi, cos phi). Polynomial in fp32 (or int32 Q2.30 on the scalar core; never in fp16, never in HVX int32
lanes without a 32x32 multiply): Taylor sin x - x^3/6 + x^5/120 - x^7/5040 (truncation <= 3.2e-7) and
cos 1 - x^2/2 + x^4/24 - x^6/720 + x^8/40320 (<= 4e-8); degree-5/6 Taylor does NOT meet 2^-15 (3.7e-5); minimax
degree 5/6 does (~1e-6). Round ONCE to Q2.14 / fp16. Table alternative: 257 Q2.14 entries (514 B),
T[j] = round(sin(j pi/512) 2^14), j = 0..256; for a 14-bit quarter index q: j = q >> 6, f = q & 63,
s = T[j] + (((T[j+1] - T[j]) f + 32) >> 6); total error < 2^-14 (use the polynomial if the 2^-15 gate budget
is required). CUDA: sincospif(k / 32768.0f). On Hexagon (no transcendental instructions anywhere) the
polynomial runs on 64 angles at once in the sentence-per-lane layout, or use exported (cos, sin) pairs (Sec. 6.3).

### 4.5 Probabilities, postselection amplification, renormalisation

Index sets: M_post = OR of (1 << q) for q in K; S = {i : (i & M_post) == 0}; class c has index b(c) =
scatter(c, S fields); N_c = re[b(c)]^2 + im[b(c)]^2; D = sum_c N_c = sum_{i in S} |psi_i|^2 (a per-lane
predicate on i, no gather). Accumulation: form re^2 + im^2 AFTER widening to fp32 (exact), reduce per lane
then tree (warp shuffle: 5 levels; HVX: 5 vror + vadd; Adreno: sub_group_reduce_add): error
<= (n/L - 1 + log2 L) u32 -- fp16 sequential would be 0.5 worst at W = 10 and stalls beyond W = 11. Q1.15 path:
squares in Q2.30 int32, sum exact and < 2^31 whenever ||hat_psi||^2 < 2 (always where the table below gives
D_max >= 1; defensively convert lane partials to fp32 or int64 before the horizontal reduce).

Postselection amplifies error: with P the projector, p_surv = ||P psi||^2 and computed hat_psi = psi + delta,

    ||hat_psi_post - psi_post|| <= 2 ||delta|| / sqrt(p_surv);   |delta P_c| <= 4 ||delta|| / sqrt(p_surv)      (4.7)

Every halving of p_surv costs half a bit; m postselected qubits in a generic state (p_surv ~ 2^-m) multiply the
forward error by 2^{m/2}. Rules: (1) N_c and D in fp32/int32 even when amplitudes are fp16/Q15; (2) never
renormalise the vector -- P_c = N_c / D is scale-invariant (two accumulators, zero extra memory);
renormalisation removes only the radial error component and costs two passes plus a synchronisation; the only
case is a continued circuit after a mid-circuit projection: on the Q1.15 path convert to fp32, multiply by
1/sqrt(p) (scalar), round once back; (3) flag p_surv < p_min = (4 ||delta||_est / tol)^2 and re-run those
sentences in fp32 (fp16 pure, Ng = 50 typical: p_min = 0.31 at tol 0.05; Q1.15 W = 10: p_min = 0.05). Log
p_surv per sentence: its corpus distribution is the format-selection statistic. Norm drift after Ng gates:
|drift of ||psi||^2| <= 2 Ng g + (Ng g)^2. Saturation on Q1.15 clips a single component at 1 - 2^-15 (error
3e-5): keep one saturation counter per launch; renormalise only if it reports repeated clipping.

### 4.6 Decision table

Per-gate state error g (worst) and g_typ; output-probability error E = A Ng g (worst) or ~A sqrt(Ng) g_typ
(typical); A = 2 without postselection, A = 4/sqrt(p_surv) with (12.6 at p_surv = 0.1). Ng counts arithmetic
gates (permutations free); a fused 4x4 counts 2.5 on fp rows, 1 on fixed-point rows. D_max = floor(...).

    unit / format                                  W      g worst   g_typ    Ng_max E<=0.01,A=2 (worst/typ)  Ng_max E<=0.05,p>=0.1 (worst/typ)
    GPU fp32 CUDA cores                            any    6.0e-7    1.2e-7   8333 / >1e6                      6613 / >1e6
    fp16 STORE, fp32 compute, fp32 gate entries    any    4.9e-4    2.8e-4   10 / 318                         8 / 200
      (CUDA cores; Adreno fp32; HVX v68+ IEEE sf)
    same with fp16 gate-entry vectors (HVX hf)     any    1.2e-3    6e-4     4 / 69                           3 / 43
    pure fp16 arithmetic (any unit)                any    4.9e-3    9.8e-4   1 / 26                           0 / 16
    bf16                                           any    3.9e-2    7.8e-3   0 / 0                            0 / 0   (do not use)
    HVX int16 Q1.15, int32 accumulate              8      4.3e-4    2.1e-4   11 / 567                         9 / 357
    HVX int16 Q1.15, int32 accumulate              10     7.8e-4    4.05e-4  6 / 152                          5 / 96
    HVX int16 Q1.15, int32 accumulate              12     1.5e-3    8.0e-4   3 / 39                           2 / 24
    HVX int16 Q1.15, int32 accumulate              16     5.6e-3    3.2e-3   0 / 2                            0 / 1
    scalar/GPU int32 Q1.31, int64 accumulate       16     8.5e-8    4.9e-8   58823 / >1e6                     46680 / >1e6
    scalar/GPU int32 Q1.31, int64 accumulate       24     1.4e-6    8e-7     3571 / >1e5                      2834 / >1e4
    tensor core / HMX fp16, fused 16x16 or 32x32   any    ~2.4e-3   ~1e-3    2 / 25   (UNVERIFIED accum.)     1 / 15

Reading. A lambeq-style sentence circuit (W = 3..10, Ng = 20..60 after fusion) with p_surv >= 0.1 is inside the
Q1.15 TYPICAL tolerance E <= 0.05 (W = 10, Ng = 60: E_typ = 0.023 at p_surv = 0.3, 0.040 at 0.1, 0.087 at
0.02 -- the last FAILS and is exactly where the p_min flag fires); the worst-case column is exceeded for
Ng > 5, so Q1.15/fp16 inference must be validated against the fp32 reference on held-out data and low-p_surv
sentences re-run in fp32. Q1.15 wins for W <= 9 or wherever the vector unit lacks fp (HVX v6x);
fp16-store/fp32-compute wins for W >= 10 on any unit with fp32 lanes; pure fp16 arithmetic and bf16 are never
the right choice; training needs fp32 or Q1.31. Because the fp16/Q15 forward pass differs from fp32 by up to
Ng g, either validate the quantised model or train with inference rounding emulated in the forward pass while
keeping fp32 gradients (quantisation-aware training). The fp32 reference is cheap: N <= 2^12 complex numbers.
The table applies to CIRCUIT-FORM inference (bent-wire, Sec. 1.4) with Ng = the fused gate count; the fused
tensordot (Sec. 1.3), whose intermediates are not unit vectors, is bounded separately in Sec. 4.7.

### 4.7 Error and range of the fused tensordot (the recommended inference kernel)

Sec. 4.1-4.6 bound unitary butterflies. The tensordot of Sec. 1.3 is a chain of contractions whose intermediates
R are NOT unit vectors, so both fixed-point range and error propagation need their own proof.

Range. Every open tensor T of Sec. 1.3 satisfies ||T||_2 <= 1 (2-norm over all entries). Induction: T = phi_{w_1}
has norm 1. Step: view T as a matrix (restT rows x cup columns) and phi_w as (cup rows x restW columns), so
R[rT, rW] = sum_k T[rT, k] phi_w[k, rW]. For fixed (rT, rW), Cauchy-Schwarz over k gives |R[rT, rW]|^2 <=
(sum_k |T[rT, k]|^2)(sum_k |phi_w[k, rW]|^2); summing over rT and rW gives

    ||R||^2 <= ||T||^2 ||phi_w||^2 <= 1        (this is ||A B||_F <= ||A||_F ||B||_F)                        (4.8)

Hence |Re R|, |Im R| <= 1 for every intermediate and for sigma (Z = ||sigma||^2 <= 1): Q1.15 needs no scale
factor anywhere in the kernel, and the int32 accumulator of one output entry (2^q complex products of two Q1.15
numbers, each product exact in Q2.30) satisfies |acc_re| = |sum_k (x_re y_re - x_im y_im)| <= sum_k |x_k| |y_k|
<= ||T[rT, .]|| ||phi_w[., rW]|| <= 1, i.e. |acc| <= 2^30 in Q2.30, for ANY cup width q (same for acc_im). Store:
n' = sat_int16((acc + 2^14) >> 15) -- round to nearest, saturate (the saturation fires only on the exactly
representable-minus-one boundary, error <= 2^-15, as in Sec. 4.3).

Propagation. sigma is multilinear in the word states, and every partial map (all operands fixed but one) has
operator norm <= 1 by (4.8): a perturbation delta phi_w produces ||delta R|| <= ||T|| ||delta phi_w|| <=
||delta phi_w||, and a perturbation of T (or of a stored R) propagates through every later step with factor
<= 1. Errors therefore ADD linearly, exactly as in Sec. 4.1:

    ||delta sigma|| <= sum_w ||delta phi_w|| + sum_steps ||delta_store(step)||,   |delta P_c| <= 4 ||delta sigma|| / sqrt(Z)   (4.9)

(the second inequality is (4.7) with p_surv = Z, since P_c = |sigma_c|^2 / Z is a postselected probability).

Q1.15 path (words generated in fp32 and narrowed ONCE; products exact; int32 sums exact; one rounding per stored
intermediate): ||delta phi_w|| <= sqrt(2 x 2^{Q_w}) 2^-16 = 2^{Q_w/2 - 15.5}; a stored intermediate with |R|
complex entries: ||delta_store|| <= sqrt(2 |R|) 2^-16.
    (1,1) N TV N: 2 x 2^-15 (two nouns, Q = 1) + 2^-14 (verb, Q = 3) + sqrt(8) 2^-16 (R, 4 entries)
                  + sqrt(4) 2^-16 (sigma, 2 entries) = 1.96e-4 worst;
                  |delta P_c| <= 4 x 1.96e-4 / sqrt(Z) = 2.1e-3 at Z = 0.1403 (Sec. 8), 6.3e-3 at Z = 2^-6.
    (2,1) N TV N: 2 x 2^-14.5 + 2^-13 + sqrt(16) 2^-16 (R, 8 entries) + sqrt(4) 2^-16 = 3.0e-4 worst;
                  |delta P_c| <= 9.6e-3 at Z = 2^-6.
    Typical (estimate, not a bound: each rounding uniform in [-2^-16, 2^-16], terms in quadrature): (1,1)
    ~5.3e-5, |delta P_c| ~ 1.7e-3 at Z = 2^-6.
    Compare circuit-form Q1.15 on the same sentence: Ng = 7 fused gates at W = 3, g_Q15(3) = 2^-14 + 8.6e-5 =
    1.47e-4, |delta P_c| <= 4 x 7 x 1.47e-4 / sqrt(0.1403) = 1.1e-2 -- five times worse than the tensordot,
    because the tensordot rounds 5 times and the circuit 7 times with per-gate coefficient quantisation on top.

fp16-store / fp32-compute path (fp16 x fp16 exact in fp32; fp32 accumulation error <= 2^q u32, negligible):
per-component RELATIVE rounding u16 = 2^-11 = 4.9e-4 on every store, so ||delta x|| <= u16 ||x|| <= u16 per
stored object (words included, since they are narrowed once):
    (1,1) and (2,1) N TV N: 3 words + 2 stores (R, sigma) -> ||delta sigma|| <= 5 u16 = 2.4e-3 worst
                  (W-independent); |delta P_c| <= 4 x 2.4e-3 / sqrt(2^-6) = 0.078 -- FAILS a 0.05 tolerance at
                  the D = 2^-6 floor; typical (RMS 0.58 u16 sqrt5) 6.3e-4 -> 0.020.
    Therefore on the fp16-store path require D >= 2^-4 (worst |delta P_c| <= 4 x 2.4e-3 / 0.25 = 0.039), or
    prefer Q1.15 at (1,1) and (2,1), where it is 12x more accurate (1.96e-4 vs 2.4e-3); the D >= 2^-6 floor of
    Sec. 6.2 is adequate for Q1.15 only. Neither path needs a scale factor (range proof above).
    If the whole tensordot is register-resident in fp32 (CUDA Layout A, Adreno fp32 fibers) with no
    intermediate store, only the word narrowings remain: 3 u16 = 1.5e-3 (fp16 words) or 1.2e-4 (Q1.15 words),
    or 3 x 8 u32 ~ 1.4e-6 if the words stay fp32 -- the fp32 reference of T6.

---------------------------------------------------------------------------------------------------------------

## 5. Learning

### 5.1 Objective

Sentence circuit (bent-wire form, Sec. 1.4) U(theta) = U_Ng ... U_1 on W qubits; K = postselected qubits,
S = sentence qubits, W = |K| + q_s. Diagonal projectors O_c[b,b] = 1 iff b == b(c); Pi_K = sum_c O_c.

    N_c = re[b(c)]^2 + im[b(c)]^2,   D = sum_c N_c  (in [0,1], = p_surv),   P_c = N_c / D  (sum_c P_c = 1)     (5.1)
    L_CE = -sum_c y_c ln(P_c + eps), g_c = dL/dP_c = -y_c/(P_c + eps);   L_MSE = sum_c (P_c - y_c)^2, g_c = 2(P_c - y_c)
    Guard: if D < eps_D (1e-7 fp32) skip the sentence or clamp D := eps_D. Clip the gradient norm at 1.

|dP_c/dtheta| <= 1/D (in fact (1 + P_c)/(2D)): small D at random init (E[D] = 2^{-|K|}) makes CE gradients spiky.

### 5.2 Parameter-shift rule, with the quotient rule

For a gate exp(-i theta G/2) with G^2 = I (Rx, Ry, Rz, any Pauli string), any expectation f(theta) =
<phi| U^dag A U |phi> (|phi> = state entering the gate) expands via U = cI - i s G to
f = a + b cos theta + d sin theta, hence EXACTLY

    df/dtheta = [ f(theta + pi/2) - f(theta - pi/2) ] / 2                                                   (5.2)

This applies to N_c(theta) and D(theta) separately (both are projector expectations), NOT to the ratio P_c nor
to the loss (verified: on the worked example the naive shift on P_0 gives -0.0151 where the true derivative is
+0.0702). Quotient rule:

    N_c' = [N_c(+) - N_c(-)]/2,  D' = sum_c N_c',  dP_c/dtheta = (N_c' - P_c D') / D,  dL/dtheta = sum_c g_c dP_c/dtheta   (5.3)

Cost: 2 P + 1 forward passes per sentence; memory: one state vector + (4 2^{q_s} + 3) fp32 scalars (save the
unshifted theta and restore by assignment, not by +-pi/2 arithmetic). Gradient error inherits 1/p_surv (not
1/sqrt) through the quotient (Sec. 4.5).

CONTROLLED ROTATIONS (CRz in IQP, CRx in Sim14) have generator |1><1| (x) P/2 with eigenvalues {0, +1/2, -1/2},
so f contains cos(theta/2), sin(theta/2) terms and (5.2) is WRONG for them (worked example: 2-term gives
-0.0685 for dN_0/dtheta where the truth is -0.0484). Use either the exact four-term rule

    df/dtheta = d1 [f(t + pi/2) - f(t - pi/2)] + d2 [f(t + 3pi/2) - f(t - 3pi/2)],
    d1 = (sqrt2 + 1)/(4 sqrt2) = 0.4267767,  d2 = -(sqrt2 - 1)/(4 sqrt2) = -0.0732233                        (5.4)

(derived from matching -omega sin(omega t) for omega = 1 and 1/2; verified numerically), or the exact
decompositions

    CRz(theta)(c->t) = [Rz_t(theta/2)] CNOT(c->t) [Rz_t(-theta/2)] CNOT(c->t)      (product order; residual 2e-16)
    CRx(theta)(c->t) = [Rx_t(theta/2)] CZ [Rx_t(-theta/2)] CZ         (Z Rx(-a) Z = Rx(a); CNOT does NOT work for CRx
                                                                        since X commutes with Rx -- that gives I)       (5.5)

and apply (5.2) to the two half-angles as independent parameters with chain-rule factors +1/2 and -1/2 (4
passes). A parameter shared by r gate occurrences (a repeated word) is the sum of the r single-occurrence
shift derivatives (2r passes); for parameters inside an effect word U^T (C11) the Ry angles enter negated
(factor -1). Prefer adjoint differentiation (Sec. 5.3), which needs none of this.

### 5.3 Adjoint (reverse-mode) differentiation -- the recommended training rule

|psi_k> = U_k ... U_1 |0>, |lambda_k> = U_{k+1}^dag ... U_Ng^dag O |psi_Ng>, f = <lambda_k|psi_k> for all k.
For U_k = exp(-i theta_k G_k/2) (G_k need NOT square to I):

    df/dtheta_k = 2 Re <lambda_k| dU_k/dtheta_k |psi_{k-1}> = Im <lambda_k| G_k |psi_k>                         (5.6)
    recurrences: |psi_{k-1}> = U_k^dag |psi_k>,  |lambda_{k-1}> = U_k^dag |lambda_k>                         (5.7)

For the postselected loss, with g_c, P_c, D frozen: dL/dtheta_k = d/dtheta_k <psi| O_eff |psi>,

    O_eff = (1/D) sum_c (g_c - gbar) O_c,  gbar = sum_c g_c P_c;   lambda_Ng[b(c)] = w_c psi_Ng[b(c)], w_c = (g_c - gbar)/D, 0 elsewhere   (5.8)

    // adjoint gradient for one sentence; A = psi, B = lambda, C = scratch (or stream G_k as a signed permutation: 2 vectors)
    A = |0..0>;  for k = 1..Ng: A = apply(U_k, A)
    compute N_c, D, P_c, g_c, gbar, w_c from A;  B = 0;  for c: B[b(c)] = w_c * A[b(c)]
    for k = Ng down to 1:
        if parameterised(k): C = apply_generator(G_k, A);  grad_local[k] = Im( sum_b conj(B[b]) * C[b] )   // fp32 reduction
        A = apply(U_k^dag, A);  B = apply(U_k^dag, B)

Sign of effect-word Ry gradients. grad_local[k] is the derivative with respect to the angle the gate was APPLIED
with (the value fed to (0.1)-(0.3), (0.7), (0.8)). Each gate k carries sign_k = -1 if it is an Ry inside a
TRANSPOSED effect word (C11: applied angle = -theta_w[i]), else +1: Rx, Rz, H, CNOT, CZ, CRz, CRx are symmetric
and enter U^T with their own angle, and every gate keeps its own generator G_k inside U^T (C13). The chain rule
dL/dtheta_w[i] = sign_k dL/d(applied angle) is applied at the scatter-add (Sec. 5.5). Omitting it silently
flips the sign of every effect-word Ry gradient in Sim14 / Sim15 / Ry-only models; IQP and Rx-Rz-Rx words are
unaffected (no Ry). The generator application below is on the applied gate, so nothing else changes.

Generators on index b: (X_q psi)[b] = psi[b ^ (1<<q)]; (Y_q psi)[b] = (bit_q(b) ? +i : -i) psi[b ^ (1<<q)];
(Z_q psi)[b] = (bit_q(b) ? -1 : +1) psi[b]; controlled |1><1|_c (x) P_t: (P_t psi)[b] if bit_c(b) = 1 else 0.
U^dag: rotations with negated angle; H, CNOT, CZ self-inverse. Only UNITARIES are in the gate list (the
postselection is entirely inside Pi_K / O_c and is never a gate; the table shapes need no Bell-basis change).
Cost ~3-4 forward passes for ALL P parameters; memory 3 (or 2) state vectors: 24 2^W B fp32 (W = 3: 192 B;
W = 5: 768 B; W = 11: 48 KB). Backward round-off O(Ng u32), negligible for Ng < 1e3; never do it in fp16.
Verified numerically: all 8 gradients of the worked example (Sec. 8) agree with central differences to 1e-7.

### 5.4 SPSA (forward-only devices)

Delta_i in {-1,+1} i.i.d.; ghat_i = [L(theta + c_k Delta) - L(theta - c_k Delta)] / (2 c_k Delta_i);
theta -= a_k ghat; a_k = a/(A + k + 1)^0.602, c_k = c/(k+1)^0.101, A ~ 10 % of planned iterations (Spall's
constants). Memory: angles + the PRNG seed (regenerate Delta) + 2 scalars. Central-difference optimum is
c ~ eps_mach^{1/3}: fp16 forward: 0.08 rad (use 0.05-0.15); fp32: irrelevant. With stored (cs, sn) =
(cos phi, sin phi): cos(phi +- c/2) = cs cc -+ sn sc, sin(phi +- c/2) = sn cc +- cs sc, 4 MACs per parameter,
no trig. Heuristic: the estimator variance is ~(P-1) mean_j (dL/dtheta_j)^2 / c^2, so expect O(P) more steps
than adjoint; use only where Sec. 5.3 is unavailable.

### 5.5 Parameter sharing, counts, optimiser, initialisation

Each parameterised gate k carries (occurrence j, local index i); sentence-flat index = word_param_base[j] + i;
global index = param_offset[w_j] + i. Sharing is the chain rule through concatenation:
grad[param_offset[w] + i] += sign_k * grad_local[k] / B (sign_k of Sec. 5.3: -1 for an Ry in a transposed
effect word, else +1) over all k with word(k) = w (fp32 atomicAdd; function words
collide B-way -- reduce per block first if it matters; atomics are order-nondeterministic, use a segmented
reduction for bitwise reproducibility). Counts: n_w = 1: P_w = 3; IQP L(n_w - 1); Sim15 2 L n_w; Sim14
4 L n_w. Example V = 1000 (600 nouns, 250 TV, 100 ADJ, 50 IV), L = 1: Sim14 6000, IQP 2450 parameters.
Adam (per parameter, fp32; t = step count from 1; g = the batch-mean gradient after norm clipping at 1, Sec. 5.1):

    m = b1 m + (1 - b1) g;   v = b2 v + (1 - b2) g^2;   m_hat = m / (1 - b1^t);   v_hat = v / (1 - b2^t);
    theta -= lr * m_hat / (sqrt(v_hat) + eps);        b1 = 0.9, b2 = 0.999, eps = 1e-8, lr = 0.01-0.1 rad.       (5.9)

Keep b1^t and b2^t as two running fp32 products (pow1 *= b1 each step; fine for t < 1e5). State m, v fp32
(8 B/param) + theta fp32 + grad fp32 = 16 B/param; 6000 params -> 96 KB. theta is the UNWRAPPED fp32 angle
(Sec. 5.6); m and v depend only on gradients, which are 4 pi-periodic in theta, so wrapping theta mod 4 pi at
export leaves them valid. Zero-initialise m, v; one Adam kernel over P_total after each batch (Sec. 6.1).
Init: uniform [0, 2 pi) with a fixed seed (better-conditioned D), or N(0, 0.1^2) (predictable early D).
Barren plateaus (Var ~ 2^-W, |grad| ~ 2^{-W/2}) are a W >~ 20-30 phenomenon (heuristic) and do not bind at
W <= 12; the real hazard is small D.

### 5.6 Periodicity and wrapping

Single-qubit rotations: theta + 2 pi gives a global phase -> loss 2 pi-periodic. Controlled rotations: exp(-i pi
G) = Z_control (x) I for G = |1><1| (x) P, so CRx(theta + 2 pi) = Z_c CRx(theta): a RELATIVE phase; the loss is
only 4 pi-periodic (worked example: P_0(theta) = 0.5826, P_0(theta + 2 pi) = 0.8764, P_0(theta + 4 pi) =
0.5826 for the verb's first CRz). Never wrap controlled angles mod 2 pi. The half-angle-turn encoding of C5
(phi = theta/2 mod 2 pi) is 4 pi-periodic in theta and therefore correct for every gate type; the (cos phi,
sin phi) pair likewise. Wrapping mod 4 pi at any time changes neither loss nor gradient, so Adam state stays
valid. Wrap only at export.

---------------------------------------------------------------------------------------------------------------

## 6. Train on GPU, infer on NPU

### 6.1 Training (CUDA, fp32, adjoint)

Bucket the batch by (shape, W); one warp (W <= 6 two-per-thread; W <= 8 register-blocked, 3 x 16 = 48
registers) or one block (W <= 11) per sentence, Layout B / B-blocked / C of Sec. 3.1;
state vectors A, B, C in registers (Layout B) or dynamic shared memory (Layout C); gate list in __constant__
memory. Per sentence: adjoint pass -> grad_local[k] -> scatter-add into the flat gradient table -> one Adam
kernel over P_total. Device memory: B 24 2^W B shared (transient) + 16 P_total + B Ng sizeof(gate_entry) +
B 4 P_s (grad_local) + token stream; W = 11, B = 256, P_total = 6000: 12 MB shared in flight, 96 KB resident,
< 1 MB tables.

### 6.2 Inference (Hexagon / Adreno, forward only)

Fused tensordot kernel (Sec. 1.3) in the per-target layout of Sec. 3.2/3.4; Q1.15 with int32 accumulation on
HVX (any generation) or fp16-store/fp32-compute on v68+ IEEE / Adreno; N_c, D in int32/fp32; report D (= p_surv)
next to P_c so the caller can reject low-D sentences (require D >= 2^-6 on the Q1.15 path and D >= 2^-4 on the
fp16-store path, else fall back to fp32). Propagated bound (Sec. 4.7, (4.9)): Q1.15 at (1,1): ||delta sigma|| <=
1.96e-4, |delta P_c| <= 4 x 1.96e-4 / sqrt(D) = 6.3e-3 at D = 2^-6 (9.6e-3 at (2,1)); fp16-store / fp32-compute:
||delta sigma|| <= 5 u16 = 2.4e-3, |delta P_c| <= 0.078 at D = 2^-6 (worst) and 0.039 at D = 2^-4 -- hence
the two D floors. No scale factor is needed on either path (every intermediate has 2-norm <= 1, (4.8)). Optional on-device fine-tuning: SPSA (Sec. 5.4) only where the Sec. 4.6 table leaves the
probability error an order of magnitude below the gradients of interest.

### 6.3 Export format (all multi-byte fields little-endian; offsets in bytes)

    Header (32 B):
      @0  magic: bytes 'Q','N','L','P' (0x51 0x4E 0x4C 0x50; as LE u32 = 0x504C4E51)
      @4  version u16;  @6 flags u16: bit0 = angle encoding (0: uint16 half-angle turns, 1: Q15 cos/sin pairs),
                                      bit1 = cup pairing (0 component-wise, 1 nested), bit2 = real-only model
      @8  V u32;  @12 P_total u32;  @16 q_n u8;  @17 q_s u8;  @18 L u8;  @19 ansatz_id u8 (C10);  @20 reserved[12]
    Word record (20 B each, packed, V records, starting @32):
      @0  word_id u32 (index into an external string table)
      @4  type_sig u8[6]: per atom bits 0-1 base (0 = n, 1 = s), bits 2-4 adjoint (0 none, 1 .l, 2 .r, 3 .ll, 4 .rr);
          0xFF = end; qubits are laid out in type_sig order (C7); > 6 atoms unsupported in this version.
          Byte j = atom j of the type string, LEFT TO RIGHT (so byte 0 is the lowest qubit field, C7). Encoding
          of (b, z) of Sec. 1.1: base = (b == S) ? 1 : 0; adjoint code a from z: z = 0 -> 0, z = -1 (.l) -> 1,
          z = +1 (.r) -> 2, z = -2 (.ll) -> 3, z = +2 (.rr) -> 4; byte = base | (a << 2). Decode:
          z = (a == 0) ? 0 : ((a & 1) ? -((a + 1) / 2) : (a / 2)). Example transitive verb n^r s n^l: bytes
          0x08 (n, code 2), 0x01 (s, code 0), 0x04 (n, code 1), 0xFF, 0xFF, 0xFF; intransitive verb n^r s:
          0x08, 0x01, 0xFF..; adjective n n^l: 0x00, 0x04, 0xFF..; adverb s^r s: 0x09, 0x01, 0xFF..
      @10 n_qubits u8;  @11 ansatz_id u8 (per-word override, 0xFF = header default)
      @12 P_w u16;  @14 param_offset u32 (entry index into the angle table);  @18 reserved u16
    Angle table (starting @ 32 + 20 V):
      flags.bit0 = 0: P_total x uint16  k, phi = theta/2 = 2 pi k / 65536  (2 B/param)        -- default
      flags.bit0 = 1: P_total x (int16 round(cos phi * 32767), int16 round(sin phi * 32767))  (4 B/param; no trig on device)

Sizes, V = 1000: Sim14 (6000 params) 32 + 20000 + 12000 = 32,032 B (pairs: 44,032 B); IQP (2450) 24,932 B.
Export quantisation: theta -> k = round(theta / (4 pi) * 65536) mod 65536 (from the UNWRAPPED fp32 angle;
4 pi-periodic, so controlled rotations are safe); d_phi <= 4.8e-5. The Q15 pair encodes the sign of cos phi
correctly by construction. Do not store fp16 angles (Sec. 4.2). The whole table (100 KB class) is loaded once
into VTCM / shared memory / L2; per-sentence off-chip traffic is word ids (2-4 B each), the label, and
2^{q_s} + 1 outputs.

---------------------------------------------------------------------------------------------------------------

## 7. Memory budget: the "almost zero memory" claim, end to end

Bytes per amplitude: complex64 (two fp32) 8; split fp16 or Q1.15 complex 4; real ansatz halves both.

    Object                                        formula                   (1,1) fp32 / fp16-Q15    (2,1) fp32 / fp16-Q15
    MODEL (parameters), V = 1e4, mean P_w = 5     V mean(P_w) x 2 B         100 KB (uint16 turns)     same; 200 KB as cos/sin pairs
    per-sentence angles, N TV N, L = 1            (sum_w P_w) x 2 B         (3+2+3) x 2 = 16 B        (1+4+1) x 2 = 12 B
    materialised unitaries                        0                         0                         0
    gate scratch (one fused 2x2 / 4x4)            8 / 32 scalars            32 / 128 B                32 / 128 B
    sincos table (optional)                       0 (polynomial) / 514 B    0                         0
    fused tensordot working set (Sec. 1.3)        2^{2qn+qs}+2^{qn}+2^{qn+qs}+2^{qn} amplitudes
                                                  16 amps (10 in place,     128 B / 64 B (80 / 40)    48 amps (36): 384 B / 192 B (288 / 144)
                                                  slot rule of Sec. 1.3)
    bent-wire circuit state, N TV N (Sec. 1.4)    2^{2qn+qs}                8 amps: 64 B / 32 B       32 amps: 256 B / 128 B
    bent-wire, ADJ N TV N / N IV ADV              2^{3qn+qs} / 2^{qn+2qs}   16 / 8 amps               128 / 16 amps
    early-cup monolithic peak, N TV N (Sec. 2.3)  2^{3qn+qs}+2^{2qn+qs}     24 amps: 192 B / 96 B     160 amps: 1.25 KB / 640 B
    naive monolithic 2^W, N TV N                  2^{4qn+qs}                32: 256 B / 128 B         512: 4 KB / 2 KB
    naive monolithic, ADJ N TV N                  2^{6qn+qs}                128: 1 KB / 512 B         8192: 64 KB / 32 KB  (not register-resident anywhere)
    naive monolithic, ADJ N TV ADJ N              2^{8qn+qs}                512: 4 KB / 2 KB          131072: 1 MB / 512 KB (fits no on-chip memory)
    permute-control constants (HVX only)          6 x 128 B                 768 B in VTCM             768 B
    CUDA exchange buffer (Layouts B/C only)       S(W)                      0 (Layout A/B)            0 / S(W)
    training: 3 state vectors, bent-wire W        24 x 2^W B                W=3: 192 B; W=4: 384 B    W=5: 768 B; W=7: 3 KB
    training: parameters + Adam + grad            16 x P_total              96 KB (6000 params)       96 KB
    outputs per sentence                          (2^{qs} + 1) x 4 B        12 B                      12 B

    State bytes S(W) = 2 x 2^W x bytes/real for reference:  W=6: 512 B fp32 / 256 B fp16;  W=8: 2 KB / 1 KB;
    W=10: 8 KB / 4 KB (= the whole HVX register file);  W=11: 16 KB / 8 KB;  W=12: 32 KB / 16 KB (= Adreno local memory, fp32);
    W=14: 128 KB / 64 KB.

Conclusion. Parameter storage is 2 bytes per angle and nothing else; no unitary, weight table or word vector is
ever stored; per-gate scratch is <= 128 B; the per-sentence working set of the recommended kernel is 40-400 B
and lives in one thread's registers (CUDA), one HVX context's registers (Q1.15, (1,1)) or one lane of a vector
pair ((2,1)); off-chip traffic per sentence is tens of bytes in and 12 B out. The irreducible object is the
2^{W_hw} state of the largest word (8-128 amplitudes here); "almost zero memory" means zero weight tables and
zero materialised unitaries, not zero state. The monolithic form at (2,1) with adjectives is out of budget on
every target and is not an option.

---------------------------------------------------------------------------------------------------------------

## 8. Worked example: "Alice loves Bob", q_n = q_s = 1, IQP (ansatz 0, L = 1), W = 3

Parse. Types: Alice n [(N,0)]_0; loves n^r s n^l [(N,+1)]_1 [(S,0)]_2 [(N,-1)]_3; Bob n [(N,0)]_4. Stack run:
p=0 push; p=1 (N,+1) with top (N,0): cup (0,1); p=2 push; p=3 (N,-1) push; p=4 (N,0) with top (N,-1): cup
(3,4). Stack = [2], type (S,0): grammatical. cups = {(0,1), (3,4)}, s position 2.

Fields (monolithic, W = 5): off_Alice = 0, off_loves = 1, off_Bob = 4; start(0) = 0, start(1) = 1 (n^r),
start(2) = 2 (s), start(3) = 3 (n^l), start(4) = 4; all widths 1. Verb tensor V[a,s,b] = phi_V[a + 2s + 4b].

Word states. Alice = (t0, t1, t2) = (0.3, 0.7, 1.1); Bob = (0.5, -0.4, 0.9); loves = (t0, t1) = (0.8, -0.6)
with t0 on CRz(0->1), t1 on CRz(1->2). Noun, gate list Rx(t0), Rz(t1), Rx(t2) on |0> (c_j = cos(t_j/2), s_j =
sin(t_j/2)):

    n[0] = c0 c2 e^{-i t1/2} - s0 s2 e^{+i t1/2},   n[1] = -i (c0 s2 e^{-i t1/2} + s0 c2 e^{+i t1/2})
    Alice = [ 0.71847188 - 0.31582980 i,  -0.13353070 - 0.60516052 i ]
    Bob   = [ 0.74959627 + 0.19470917 i,   0.03946950 - 0.63137622 i ]

Verb, gate list H(0), H(1), H(2), CRz(t0)(0->1), CRz(t1)(1->2) on |000>: H^{x3}|000> = 2^{-3/2} sum_k |k>;
CRz(0->1) multiplies index k with b0 = 1 by e^{-i t0/2} (b1 = 0) or e^{+i t0/2} (b1 = 1); CRz(1->2) multiplies
b1 = 1 by e^{-+ i t1/2} according to b2. Hence

    V[k] = 2^{-3/2} exp( i [ b0 (2 b1 - 1) t0/2 + b1 (2 b2 - 1) t1/2 ] ),   |V[k]| = 2^{-3/2} = 0.35355339 for all k
    phases (rad), k = 0..7:  0, -0.4, +0.3, +0.7, 0, -0.4, -0.3, +0.1
    e.g. V[3] (b0=b1=1, b2=0): +t0/2 - t1/2 = 0.4 + 0.3 = 0.7;  V[6] (b1=b2=1, b0=0): +t1/2 = -0.3.

Sentence (1.5): sigma[s] = sum_{a,b in {0,1}} Alice[a] V[a + 2s + 4b] Bob[b]  (4 complex MACs per output):

    sigma = [ -0.02640234 - 0.28465252 i,   0.08196341 - 0.22764750 i ]
    Z = 0.14026553,   p = [ 0.58263882, 0.41736118 ],   Loss(y=0) = 0.54018780,  Loss(y=1) = 0.87380330

Bent-wire circuit (W = 3, the hardware form and the training circuit). Qubits: 0 = verb n^r, 1 = verb s,
2 = verb n^l. Gate list: H(0) H(1) H(2) CRz(0.8)(0->1) CRz(-0.6)(1->2)  [verb, Ng = 5];
then U_Alice^T on qubit 0 = reversed list with symmetric gates unchanged: Rx(1.1) Rz(0.7) Rx(0.3);
then U_Bob^T on qubit 2: Rx(0.9) Rz(-0.4) Rx(0.5). Total Ng = 11 (after fusion: 3 H, 2 CRz, 2 fused 2x2 = 7).
K = {0, 2}, M_post = 0b101, S = {1}, b(c) = c << 1: N_0 = |psi[0]|^2, N_1 = |psi[2]|^2, D = N_0 + N_1.
Result: psi[0] = sigma[0], psi[2] = sigma[1] to 1e-16; D = p_surv = Z = 0.14026553 (no 2^{-q} factor).
Physical Bell-measurement form on W = 5 (CNOT(0->1), H(0), CNOT(3->4), H(3), all four measured 0):
psi[0] = sigma[0]/2, psi[4] = sigma[1]/2, P_post = Z/4 = 0.03506638 -- same p after normalisation.

Fused tensordot (the inference kernel): T = Alice (fields [(0,1)]); word loves: cup field (T pos 0, factor 1);
R[s,b] = sum_a Alice[a] V[a + 2s + 4b] (fields [(2,1),(3,1)], 4 amplitudes, 8 MACs); word Bob: cup (T pos 3,
factor 1); sigma[s] = sum_b R[s + 2b] Bob[b] (4 MACs). Peak = 2 + 8 + 4 = 14 amplitudes.

Gradients (fp32/fp64 reference; central differences agree to 1e-7 with the adjoint rule and with (5.3)/(5.4)),
parameter order Alice t0 t1 t2, Bob t0 t1 t2, loves t0 t1:

    dp_0/dtheta      =  0.0701761  0.0457067 -0.0415143   0.1819065 -0.1080537  0.0905236  -0.2073400 -0.5529685
    dLoss(y=0)/dtheta = -0.1204452 -0.0784477  0.0712521  -0.3122115  0.1854557 -0.1553682   0.3558636  0.9490759
    checks: Alice t0 (Rx): N_0' = 0.0549342, D' = 0.0773908, (N_0' D - N_0 D')/D^2 = 0.0701761; naive shift on p_0 = -0.0151382 (wrong).
            loves t0 (CRz): four-term N_0' = -0.0484303 (two-term would give -0.0684908, wrong); dp_0 = -0.2073400.
            periodicity: p_0(loves t0 + 2 pi) = 0.87639280 != p_0; p_0(+ 4 pi) = 0.58263882 = p_0; p_0(Alice t0 + 2 pi) = p_0.

Half-angle turn encoding of loves t0 = 0.8: k = round(0.8 / (4 pi) 65536) = round(4172.15) = 4172; decode
phi = 2 pi 4172 / 65536 = 0.399985 rad, error |0.4 - 0.399985| = 1.45e-5 rad < the 4.8e-5 bound of C5. Memory for this sentence: 8 angles = 16 B in; working set 14 complex = 112 B fp32 / 56 B Q15; 12 B out.

---------------------------------------------------------------------------------------------------------------

## 9. Implementation checklist

Kernels / modules (one compile-time instantiation per (shape, q_n, q_s, ansatz, L)):
 1. Lexicon + shift-reduce parser -> (cups, s position) -> field map start(p), width(p) -> PROGRAM (Sec. 2.6),
    with fusion and the state/effect assignment of Sec. 1.4 done at program-compile time. Host side, once per shape.
 2. Angle codec: uint16 half-angle turns <-> fp32; octant reduction + polynomial (Sec. 4.4); optional Q15 pairs.
 3. Word-state generator phi_w from theta_w for ansatz 0-4 (C10), in registers, fp32; Q15/fp16 narrowing once.
 4. Butterflies: apply_1q (2.1)-(2.2), diagonal 1q, CRz (0.7), CNOT (0.5), CZ (0.6), dense 4x4 (2.3), flip mask (2.4).
 5. Fused tensordot contract over field lists (Sec. 1.3) and the matmul form (2.8); scatter(x, fields) helper.
 6. Postselect-compact (2.5) and single-qubit (2.5'), Bell-cup gather (2.6), compact(i) (2.4), Kronecker-in (2.7)
    (statevector path only); wire_map application for effect words (Sec. 1.4 table).
 7. Readout: N_c, D, P_c with widen-before-square and tree reduction; Z guard; p_surv flag; fidelity (1.5).
 8. Adjoint gradient (Sec. 5.3) with generator application and scatter-add into the flat table; Adam.
 9. Parameter-shift (5.2)-(5.5) as a cross-check only; SPSA (5.4) for the NPU.
10. Export writer / loader (Sec. 6.3) with alignment-safe 20 B records.
11. Per-target back ends: CUDA Layout A/B/C (Sec. 3.1); HVX sentence-per-lane Q15 and amplitude-per-lane
    (Sec. 3.2); Adreno fiber-per-sentence (Sec. 3.4). Build check: 0 B local memory / spills; HVX: IEEE-fp
    feature on if fp16 is used; FTZ off for state kernels.

Tests (exact expected values from the fp64 reference of Sec. 8; tolerances: fp32 1e-6, fp16-store 3e-3, Q1.15 2e-3):
 T1 Index helpers: i0(j,k) bijective onto {bit_k = 0}, i00(j) >= j, compact(i00(j)) == j for all lo < hi at W = 6.
 T2 apply_1q / apply_2q against the dense reference matrix M of C13 (equivalently the C2 Kronecker product) for
    every k, every lo < hi at W = 6; U4 built from (0.11) for CNOT, CZ, CRz, CRx in BOTH control orientations and
    from (0.12) for lift_lo / lift_hi; CRz local diagonals for control hi and control lo per C6 / (0.11).
 T3 Gate identities: CRz decomposition (5.5) residual < 1e-15; CRx = Rx CZ Rx CZ; CRx via CNOT sandwich equals I
    (the negative test); U^T by reversed list (C11) equals the transposed dense matrix for IQP and Sim15 words.
 T4 Word states: Alice, Bob, loves, sleeps (below) amplitudes as listed; |V[k]| = 2^{-3/2} for all k under ansatz 0.
 T5 W = 2, "Alice sleeps": sleeps type n^r s, IQP L = 1, t0 = 0.8: IV = [0.5, 0.46053050 - 0.19470917 i, 0.5,
    0.46053050 + 0.19470917 i]; sigma = [0.17991068 - 0.41061012 i, 0.41557129 - 0.46260942 i];
    Z = 0.58767549; p = [0.34197193, 0.65802807]. All angles zero: sigma = [0.5, 0.5], Z = 0.5, p = [0.5, 0.5].
    Bent-wire form: IV on qubits 0 (n^r), 1 (s); U_Alice^T on qubit 0; K = {0}, S = {1}; psi[0], psi[2] == sigma.
 T6 W = 3, "Alice loves Bob": every number of Sec. 8, via (a) tensor contraction (1.5), (b) fused tensordot,
    (c) bent-wire W = 3 circuit, (d) monolithic W = 5 with Bell cups (psi = sigma/2, P_post = Z/4). All angles
    zero: sigma = [2^{-3/2}, 2^{-3/2}], Z = 0.25, p = [0.5, 0.5].
 T7 Gradients: adjoint == central FD (h = 1e-6) for the 8 parameters to 1e-6; two-term shift + quotient rule for
    Alice t0 == 0.0701761; four-term shift for loves t0 == -0.2073400; assert two-term on CRz != truth; assert
    naive shift on p_0 != truth; 4 pi periodicity of CRz vs 2 pi of Rx. For a Sim15 (Q = 2) EFFECT word in the
    bent-wire circuit (e.g. ADJ in ADJ N TV N with wire_map [1, 0]), adjoint gradient of every Ry angle == central
    FD to 1e-6 -- this checks the sign_k = -1 factor of Sec. 5.3; assert that the unsigned value has the
    opposite sign. Also: T5 / T6 bent-wire results for every row of the Sec. 1.4 wire-map table equal the
    tensordot (1.3) to 1e-12 (fp64), which checks wire_map, K, S and b(c) for all six shapes.
 T8 Numerics: unitarity ||phi_w|| = 1 +- 2 c Ng u per word; Q15 store saturates and rounds half-up on
    (acc = 2^29 sqrt2 - 1) and (-2^29); fp16 probability accumulation rejected by test (sum of 2^11 terms of 2^-11
    in fp16 stalls); Q15 vs fp32 sentence error <= g_Q15(W) Ng and P_c error <= 4 e / sqrt(D).
 T9 HVX bring-up: ramp vector through vmpy/vasr round trip is the identity permutation; vdelta xor controls for
    k = 0..5 verified against a lane-id vector; measured fl(a*b) rounding mode and subnormal behaviour recorded.
 T10 Export: write/read round trip; magic bytes; record stride 20; type_sig of n^r s n^l == 08 01 04 FF FF FF;
    angle decode error <= 4.8e-5 rad, decode(4172) = 0.399985 +- 1e-6 (Sec. 8); a
    controlled angle of 0.8 + 2 pi decodes to a DIFFERENT k than 0.8 (4 pi periodicity preserved).

---------------------------------------------------------------------------------------------------------------

## 10. Open questions and UNVERIFIED items

 1. lambeq/DisCoPy conventions if weights are ever imported: theta = 2 pi phase; CRz phase convention (symmetric
    vs diag(1,1,1,e^{i theta}) in older DisCoPy); IQP closing-H layer (assumed none, ansatz 0); Sim14/15 ring
    order; nested vs component-wise cup pairing for q >= 2; symbol order in the parameter vector. Verify each on
    one exported circuit numerically; none affects training from scratch.
 2. HVX resources: 2 multiply units / 1 permute unit per packet (LLVM model), whether hf add/sub occupy a multiply
    unit, vdelta control-byte convention, widening-multiply pair lane order and Rt half assignment, pair-producing
    shuffles as 1 or 2 ops, fp16 rounding mode (RN vs truncation) and subnormal flushing, which SoCs ship the
    IEEE-result fp ops, HVX contexts per cDSP (2-4), VTCM per SoC (256 KB - 4 MB on v66-v69; 8 MB v73+),
    VTCM retention across power collapse.
 3. HMX: reachable from the adapter at all on v68-v79 (ISA public only from V81); tile layouts; fp16 tile support
    generation (v73+?); accumulation width. Not needed by the recommended kernel.
 4. Cloud AI 100: vector unit width/register count (assumed HVX-1024 x 32); int16/fp32 support; kernel-level access.
 5. Adreno: wave size 64 vs 128 per part; register file and registers per fiber at full occupancy; native_sin/
    native_cos accuracy; local-memory vs L2 bandwidth.
 6. CUDA: cc 10.x / 12.0 shared-memory maxima (227 / 99 KB assumed); shuffle throughput (32 lanes/clk/SM assumed);
    tensor-core accumulation semantics (truncating, per Fasi et al.); register counts are ptxas-verified targets.
 7. Error constants: c = 10 (pure fp16 2x2) and c = 1 (store-only) are proven bounds; "typical" columns are
    random-walk estimates, not bounds; the fp16 FTZ term applies only if the device flushes.
 8. Design choices left open: L (>= 2 recommended for ansatz 0), real-only model (Sim15/Ry-only, halves memory,
    requires Ry-only Q = 1 words), nested vs component-wise cups (flags.bit1), whether to widen the lexicon to
    relative pronouns (requires the backtracking parser and shapes with cycles, where the tensordot order becomes
    a treewidth problem).
