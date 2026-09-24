# QNLP specification — Addendum: grammar coverage, capacity, hybrid architecture, discourse, positioning

This addendum continues qnlp_first_principles_spec.md (Sections 11-19; equation labels (11.n)..(19.n)). It obeys
Conventions C1-C13 of Section 0 verbatim; four additional conventions C14-C17 are fixed below and referenced where
used. The engineer's two criticisms are accepted as stated; what follows lifts each as far as the physics and the
memory budget allow, says where the ceiling is, and prices every lift in bytes and ops. Plain-ASCII maths,
C-like pseudocode, no Python. Claims that could not be checked in this environment are marked UNVERIFIED.
The report qnlp_hardware_limits_distilled.md does not exist in the scratchpad at the time of writing; nothing is
cited from it. Verification programs for this addendum: scratchpad/addendum/t18.c, t18b.c, mix.c (fp64, this
document), scratchpad/derivH/jsvd.c and mpstest.c (Jacobi SVD, MPS update), scratchpad/verE/frob.c, parse.c,
refE/chkE.c (parser and Frobenius contractions), derivF/*.c (capacity tables and Z statistics), scratchpad/critic/chk.c
(independent recomputation of the T12 / T15 / T19 values, the MPS capped counts and the parser bytes) and
scratchpad/critic/adjtree.c (fp64 tree adjoint of Sec. 11.7 against central differences on four sentences).

Additional conventions (C14-C17)

C14 Units and tiers. KB = 1024 B, MB = 1024 KB throughout this addendum (Sec. 7's figures already follow this).
    Memory tiers used in every table: REGISTER = one CUDA thread (<= 255 x 4 B, about 1 KB, Layout A) or the
    HVX register file (32 x 128 B = 4 KB, one context); VTCM = the 8 KB VTCM slice of Sec. 7 unless a larger
    reservation is stated explicitly ("VTCM >= X KB"; 8 MB total on v73+ per Sec. 3.2); SMEM = CUDA shared
    memory, 48 KB static, 99 / 163 / 227 KB dynamic by compute capability, and Adreno local memory 32 KB; HBM =
    GPU global memory, L2, DDR. Register residency is a property of indexing (Sec. 3.1): every claim "fits in
    registers" below presumes all indices compile-time after unrolling and is a ptxas-verified target (0 spill
    bytes), not a fact.

C15 Extended type alphabet and export format (version += 1). Basic types B = {N, S, I, Q}: I = infinitive
    clause, Q = question. q(I) = q_i, q(Q) = q_q; header @20 q_i u8, @21 q_q u8 (0 means "same as q_s").
    type_sig base codes (Sec. 6.3, bits 0-1): 0 = n, 1 = s, 2 = i, 3 = q; adjoint codes unchanged. A SPIDER word
    (relative pronoun, coordination, Sec. 11.3) is marked by the per-word ansatz_id = 0xFE: P_w = 0,
    param_offset ignored; all factors of the same basic type within the word are joined by one Frobenius
    delta, and a basic type that occurs exactly once in the word is closed by eps (all-ones). This reproduces
    (11.10)-(11.13') for who, whom, and_n, and_vp, and_s with zero stored parameters. The recogniser's accept test
    takes the target type: accept(p, target) = (types[p] == (target, 0)) && inner(0, p-1) && inner(p+1, m-1); a
    sentence has target S, a noun phrase N, a question Q. Header flags.bit3 = 1 marks the presence of the angle
    GENERATOR block of Sec. 13.7 (hybrid models) after the angle table; a word record with param_offset =
    0xFFFFFFFF, or a word absent from the word table, is "generate" (Sec. 13.7). The same version bump covers both.

C16 Word graph and node ops (the data the kernel consumes for an arbitrary sentence).
      struct Cup  { uint8_t i, j; };                       // global type positions, i < j
      struct Node { uint16_t word; uint8_t nchild; uint8_t child[4]; uint8_t cmask[4];  // bit f set: factor f of
                    uint8_t pmask; uint8_t kind; };        //   THIS word joins child k; pmask = factors toward
                                                            //   the parent (root: the survivor factor)
      // kind: 0 = unitary word (fused tensordot per child, Sec. 1.3);  1 = spider (ansatz_id 0xFE, C15, node op of
      //       Sec. 11.4);  2 = CHAIN (11.18''): child[] are the cycle members in chain order, the node's value is
      //       their matrix product with two open fields (low field = child[0]'s field toward the wrapper, high field
      //       = child[nchild-1]'s), and the WRAPPER lists the chain node as one child with q_k = q_a + q_b.
      // sizeof(Node) = 14 B (2 + 1 + 4 + 4 + 1 + 1 = 13, padded to the uint16 alignment); nchild <= 4 is asserted by
      // build_tree: the Sec. 11.2 lexicon has at most 3 children per word (ditransitive: 3; and_vp: 3; who: 2).
    tree[] is in post-order, root last. A word joined to another by two cups (and_vp -- verb) is ONE child with
    q_k = total width of both cups. The kernel is a sequence of NODE OPS, one compile-time instantiation per
    (type string, cmask, pmask) triple occurring in the lexicon (about 25 for the Sec. 11.2 lexicon), each fully
    unrolled with the word tensor in registers; inter-node result vectors (2^{q} complex each, at most one per
    word) live in SMEM / VTCM / Adreno local memory. Sentences are batched by tree signature for lane-lockstep
    targets (Sec. 2.6). The parser (Sec. 11.1) and the front end (Sec. 11.6) run on the host CPU or the DSP
    scalar core, outside the per-sentence device budget; the device receives cups[] (2 B each) and tree[]
    (14 B per word, sizeof(Node) above; a 20-word sentence: 280 B + 2 B per cup).

C17 DisCoCirc functor (text circuits, Sec. 14). Noun wire j is the q_n-qubit field [q_n j, q_n (j+1)) of a text
    register of W = q_n N qubits, wires numbered by first mention (C7: first mention lowest). A 2-wire verb gate
    is a C10 ansatz on Q = 2 q_n qubits whose LOCAL qubits 0..q_n-1 are the SUBJECT and q_n..2q_n-1 the OBJECT,
    applied through wire_map(j) = q_n i_subj + j (j < q_n), q_n i_obj + (j - q_n) (j >= q_n), exactly as
    Sec. 1.4 maps effect words (C6 orientation preserved by bit tests). When i_subj > i_obj the mapped gate list
    is still correct; a DENSE 4^{q_n} x 4^{q_n} form must instead be permuted: G'[pi(m_out)][pi(m_in)] =
    G[m_out][m_in], pi(m) = (m >> q_n) | ((m & (2^{q_n}-1)) << q_n). Per clause the gates are emitted in the
    order (1) adjective / determiner gates of the clause's argument nouns in text order, (2) verb gate(s),
    (3) adverb gates; relative pronouns and conjunctions emit no gate. The subject (object) of a verb is the
    HEAD NOUN of the phrase whose outermost n cups the verb's n^r (n^l): follow the partner of the verb's factor
    and, while the partner word is an adjective / determiner, follow that word's n^l cup. Readout = the reduced
    density matrix of a queried wire (14.19) or a final effect word on the queried fields.

---------------------------------------------------------------------------------------------------------------

## 11. Grammar coverage beyond six shapes

Scope. Section 1.1's parser and shape table are a special case of what follows. Nothing here changes C1-C13, the
fused tensordot of Sec. 1.3 or the readout; the export format changes only as stated in C15. The kernel consumes
three things: a list of words with type strings (C7 fields), a set of cups (or spider hyperedges), and the
survivor position (C16). This section is about producing that data for real sentences and about what it costs.

### 11.1 Free pregroup, recognition, the chart parser (primary) and the greedy pre-check

Simple type (b, z), b in B (C15), z in Z. Adjoints and the only rewrite needed for recognition:

    (b, z)^l = (b, z-1),  (b, z)^r = (b, z+1);  n^{ll} = (N,-2), n^l = (N,-1), n = (N,0), n^r = (N,+1), n^{rr} = (N,+2)  (11.1)
    contraction   (b, z)(b, z+1) -> 1   (adjacent types; covers t^l t -> 1 at z = -1 and t t^r -> 1 at z = 0)          (11.2)
    expansion     1 -> (b, z+1)(b, z)   (never used for recognition)                                                    (11.3)

Lambek's switching lemma: every derivation can be rearranged with all contractions before all expansions; if the
target is one simple type, the string after the contractions must already be that type (expansions only
lengthen). So a type string of length m reduces to (target, 0) iff it does so by (11.2) alone, and the cup set is
a NON-CROSSING perfect matching of the m-1 non-survivor positions, each matched pair (i, j), i < j, satisfying
b_i = b_j, z_j = z_i + 1, with the substring strictly between reducing to 1. Recognition is polynomial.

Chart recogniser (THE production parser). R[i][j] = 1 iff types[i..j] reduces to 1 (j - i odd). Position i's
partner is unique in any matching, so enumerating it counts every matching exactly once:

    R[i][j] = OR over j' in {i+1, i+3, ..., j}:  match(i, j') && inner(i+1, j'-1) && inner(j'+1, j)                   (11.4)
    match(i, j') = (b_i == b_j') && (z_j' == z_i + 1);   inner(a, b) = (a > b) ? 1 : R[a][b]
    accept(p, target) = (types[p] == (target, 0)) && inner(0, p-1) && inner(p+1, m-1)                                  (11.5)
    #parses = sum_p count(0, p-1) * count(p+1, m-1),  count(a, b) = (a > b) ? 1 : C[a][b]

    typedef struct { int8_t b; int8_t z; uint16_t word; uint8_t factor; } TypeRec;      // 5 B, padded to 6 B
    // R: bit matrix m x m (m^2/8 B); C: uint32 matchings, odd-length upper triangle only (m^2 B), SATURATING adds
    // (cap 2^32-1: CFG-generated strings reached 1.04e6 parses at m = 57, refE/genE.c); BP: first partner, uint8
    // (m^2/4 B): 1.375 m^2 B in total. m = 256: 8 KB + 64 KB + 16 KB = 88 KB (90,112 B); m = 92 (a 40-word sentence,
    // m ~ 2.3 x words, UNVERIFIED ratio): 1.375 m^2 = 11,638 B = 11.4 KB.
    // Host / DSP-scalar memory, never the device budget (C16). Time O(m^3), actual ~m^3/12 inner steps.
    for (len = 2; len <= m; len += 2)
      for (i = 0; i + len - 1 < m; i++) {
        j = i + len - 1;  R[i][j] = 0;  C[i][j] = 0;
        for (jp = i + 1; jp <= j; jp += 2)
          if (match(i, jp) && inner(i+1, jp-1) && inner(jp+1, j)) {
            R[i][j] = 1;  if (C[i][j] == 0) BP[i][j] = jp;
            C[i][j] = sat_add(C[i][j], sat_mul(count(i+1, jp-1), count(jp+1, j)));
          }
      }
    // first parse:  cups(i, j): if (i > j) return; jp = BP[i][j]; emit (i, jp); cups(i+1, jp-1); cups(jp+1, j)
    // k-th parse (mixed radix over the counts; needed by (11.25)-(11.26)):
    kth(i, j, k): if (i > j) return;
      for (jp = i+1; jp <= j; jp += 2) if (match(i,jp) && inner(i+1,jp-1) && inner(jp+1,j)) {
        nL = count(i+1, jp-1);  n = nL * count(jp+1, j);
        if (k < n) { emit (i, jp); kth(i+1, jp-1, k % nL); kth(jp+1, j, k / nL); return; }
        k -= n; }
    // survivors: for each p with accept(p, target): parses = count(0,p-1) * count(p+1,m-1); pick (p, k) likewise.

Verified (refE/chkE.c, verE/parse.c): coded verbatim, 0 false accepts and 0 false rejects against an exhaustive
search on 300k random and about 200k CFG-generated strings; every extracted cup set valid and planar.

Greedy shift-reduce (Sec. 1.1) is kept ONLY as a fast pre-check for the six shapes of Sec. 1.1, where it is exact.
It is not adequate for the lexicon of Sec. 11.2: it fails whenever a noun is followed by a post-nominal
n^r n ... word (preposition, relative pronoun, coordination, possessive) -- the pattern n^l n n^r n arises with
SINGLE adjoints -- and on VP coordination ("Alice sleeps and dreams": greedy consumes the n^r at position 1 with
(0,1) and dead-ends). Measured over CFG-generated sentences of the Sec. 11.2 lexicon greedy rejects 46-58 %; a
bounded backtracking variant (budget 64) still rejects 17-38 % (refE/chkE.c). The backtracker previously
proposed for this role also had a bug (it restored the stack pointer but not the stack contents); a correct
replay form exists (verE/parse.c parse_sr_fixed: a forced-shift bitmask, re-run greedy from 0 on each choice) but
is not the production path. Production path: parse() = chart (11.4)-(11.5). Both return the same data:
cups[] as (i, j) pairs, i < j, planar by construction, plus the survivor; position -> qubit field exactly as
Sec. 1.1: start(p) = off_word[p] + fo_word[p][factor[p]], width(p) = q(b_p).

### 11.2 Lexicon for real constructions

Type strings left to right; positions are global simple-type positions with the first word at 0; every row was
verified by the chart parser (1 parse each, cups and survivor as stated; verE/parse.c). q(I) = q_s recommended.

    class                         type                 example, cups, survivor
    determiner, adjective         n n^l                Sec. 1.1
    ditransitive verb             n^r s n^l n^l        "Alice gives Bob flowers": n | n^r s n^l n^l | n | n : (0,1) (4,5) (3,6); s at 2. Depth 2.
    pre-verbal adverb             n^r s s^l n          "Alice quickly runs": n | n^r s s^l n | n^r s : (0,1) (4,5) (3,6); s at 2. Depth 2.
    post-verbal adverb            s^r s                Sec. 1.1
    sentence-initial adverb       s s^l                "Fortunately Alice sleeps": s s^l | n | n^r s : (2,3) (1,4); s at 0
    noun-modifying preposition    n^r n n^l            "man in park": n | n^r n n^l | n : (0,1) (3,4); n at 2 (target N)
    VP-modifying preposition      s^r s n^l            "sleeps in park": n | n^r s | s^r s n^l | n : (0,1) (2,3) (5,6); s at 4
    auxiliary (Lambek form)       n^r s s^l n          "Alice does sleep": as pre-verbal adverb: (0,1) (4,5) (3,6); s at 2
    auxiliary (infinitive form)   n^r s i^l            "Alice does not sleep": n | n^r s i^l | i i^l | i : (0,1) (3,4) (5,6); s at 2
    negation, infinitive form     i i^l                as above
    negation, Lambek form         n^r s s^l n          "Alice does not sleep": n | n^r s s^l n | n^r s s^l n | n^r s :
                                                       (0,1) (4,5) (3,6) (8,9) (7,10); s at 2. (The s-free type n^r n also
                                                       parses but attaches the negation to the subject: do not use it.)
    sentential-complement verb    n^r s s^l            "Alice believes Bob sleeps": n | n^r s s^l | n | n^r s : (0,1) (4,5) (3,6); s at 2
    complementiser "that"         s s^l                "... believes that Bob sleeps": ... s^l | s s^l | n | n^r s : (3,4) (6,7) (5,8)
    noun coordination "and"       n^r n n^l            "Alice and Bob sleep": n | n^r n n^l | n | n^r s : (0,1) (3,4) (2,5); s at 6
    sentence coordination         s^r s s^l            "[S1] and [S2]": both s wires cup into the conjunction; its middle s survives.
                                                       Tensor (11.13'), formula and cost in Sec. 11.3 ("Alice sleeps and Bob dreams":
                                                       n | n^r s | s^r s s^l | n | n^r s : (0,1) (2,3) (6,7) (5,8); s at 4)
    VP coordination               s^r n^{rr} n^r s s^l n  = (n^r s)^r (n^r s) (n^r s)^l
                                                       "Alice sleeps and dreams": n | n^r s | s^r n^{rr} n^r s s^l n | n^r s :
                                                       (2,3) (1,4) (0,5) (8,9) (7,10); s at 6. Depth 3.
    subject relative pronoun      n^r n s^l n          "Alice who loves Bob sleeps": n | n^r n s^l n | n^r s n^l | n | n^r s :
                                                       (0,1) (4,5) (7,8) (3,6) (2,9); s at 10. Depth 3.
    object relative pronoun       n^r n n^{ll} s^l     "Alice whom Bob loves sleeps": n | n^r n n^{ll} s^l | n | n^r s n^l | n^r s :
                                                       (0,1) (5,6) (4,7) (3,8) (2,9); s at 10. Depth 4.
    possessive "'s"               n^r n n^l            "Alice's dog": (0,1) (3,4); n at 2
    wh-question (object gap)      q n^{ll} s^l         "whom Alice loves": q n^{ll} s^l | n | n^r s n^l : (3,4) (2,5) (1,6); q at 0.
                                                       Aux-inverted questions ("whom does Alice love") need Lambek's question
                                                       types; one consistent planar assignment is whom = q n^{ll} q^l, does =
                                                       q i^l n^l, love = i n^l: (2,3) (5,6) (4,7) (1,8), q at 0 -- UNVERIFIED
                                                       against Lambek (2008); not part of the shipped lexicon.

Long-range dependency, general statement: an iterated adjoint (b, z +/- 2) at position i cups a (b, z +/- 1) at
position j with everything strictly between reducing to 1. The distance |j - i| is unbounded; only the nesting
depth of the material in between costs anything, and only in the streaming (11.21) and circuit (11.23) forms.
Recursion. RIGHT-embedding costs constant depth: "Alice believes that Bob thinks that Carol sleeps" = n | n^r s s^l |
s s^l | n | n^r s s^l | s s^l | n | n^r s has cups (0,1) (3,4) (6,7) (5,8) (9,10) (12,13) (11,14), s at 2; the
complementiser's s cups the preceding s^l immediately, the embedded clauses are SIBLINGS, and 1, 2, 3 levels all
have nesting depth D = 2 and maximum cut width 3. CENTRE-embedding grows: one object relative has D = 4, two
stacked ("Alice whom Bob whom Carol loves loves sleeps") D = 7: +3 per object-relative level, +2 per
subject-relative level. Only the streaming and circuit forms pay for D; the bottom-up peak (11.18') does not.

### 11.3 Frobenius algebras: relative pronouns and coordination

On a q-qubit wire (dimension 2^q) the special commutative Frobenius algebra has the spider

    delta[i, j, k] = 1 if i == j == k else 0,  i, j, k in 0 .. 2^q-1  (bitwise: all q bit pairs equal, C8 component-wise)  (11.6)
    mu (merge, 2 in 1 out):  (u . v)[i] = u[i] v[i]      2^q complex multiplies (6 real flops each); in place in u        (11.7)
    Delta (copy, 1 in 2 out): u -> diag(u), never materialised: the consumer reads u[i] a second time                     (11.8)
    eps (counit): eps[i] = 1:  sum_i u[i], 2^q - 1 complex adds;   eta (unit) = all-ones vector                            (11.9)

Non-unitarity. mu: C^{4^q} -> C^{2^q} has a kernel of dimension 4^q - 2^q (it is not injective), so no unitary
implements it; on hardware it is CNOT(x_r -> y_r), r = 0..q-1, then postselect y = |0..0>:
sum_{i,j} u_i v_j |i>|i xor j> -> sum_i u_i v_i |i>, amplitude factor exactly 1. Delta is an isometry (fresh
|0> ancilla + the same CNOTs, factor 1). eps = 2^{q/2} <0|^q H^q, so each deleted q-qubit wire costs 2^{-q/2} in
amplitude (2^{-q} in p_surv) -- this is where the 2^{-q/2} of Sec. 1.4's cups reappears; it cancels in (1.4)/(1.6).

Relative pronoun and coordination tensors in C7 factor order (Sadrzadeh-Clark-Coecke construction, UNVERIFIED
citation details; the tensors below were verified against brute-force contraction, verE/frob.c and addendum/t18.c):

    who   [a_{n^r}, b_n, c_{s^l}, d_n]          = delta[a, b, d] eps[c]        who[a + 2^{q_n} b + 2^{2 q_n} c + 2^{2 q_n + q_s} d]   (11.10)
    whom  [a_{n^r}, b_n, d_{n^{ll}}, c_{s^l}]   = delta[a, b, d] eps[c]        whom[a + 2^{q_n} b + 2^{2 q_n} d + 2^{3 q_n} c]        (11.11)
    and_n [a_{n^r}, b_n, c_{n^l}]               = delta[a, b, c]                                                                 (11.12)
    and_vp[x_{s^r}, y_{n^{rr}}, y'_{n^r}, z_s, x'_{s^l}, y''_n] = delta[y, y', y''] delta[x, z, x']                              (11.13)
    and_s [x_{s^r}, z_s, x'_{s^l}]              = delta[x, z, x']     and_s[x + 2^{q_s} z + 2^{2 q_s} x']  (C15: all three factors
                                                  are S -> one delta; zero parameters)                                            (11.13')

None is stored: "who" would be 2^{4 q_n + q_s} entries (32 at (1,1), 512 at (2,1)) but is a 0/1 pattern evaluated by
index equality: zero parameters, zero bytes (C15 spider marker). Sentence formulas (V[a, s, b] = phi_V[a + 2^{q_n} s +
2^{q_n+q_s} b]; IV[a, s] = phi[a + 2^{q_n} s]):

    "Alice who loves Bob sleeps":  sigma[s] = sum_a Alice[a] ( sum_c sum_b V[a, c, b] Bob[b] ) IV[a, s]                (11.14)
    "Alice whom Bob loves sleeps": sigma[s] = sum_a Alice[a] ( sum_c sum_b Bob[b] V[b, c, a] ) IV[a, s]                (11.15)
    "Alice and Bob sleep":         sigma[s] = sum_a Alice[a] Bob[a] IV[a, s]                                            (11.16)
    "Alice sleeps and dreams":     sigma[s] = sum_a Alice[a] IV1[a, s] IV2[a, s]                                        (11.17)
    "[S1] and [S2]":               sigma[s] = sigma_1[s] sigma_2[s]   ((11.7) on the two sub-sentence vectors; the
                                   sub-sentences are evaluated as independent trees, then merged)                       (11.17')

Sentence coordination (11.13'), priced: 2^{q_s} complex multiplies; peak = max( peak(S1), peak(S2) + 2^{q_s} ) (hold
sigma_1 while S2 is evaluated; evaluate the sub-sentence with the larger peak first, as (11.18')); the merge step alone
is 3 x 2^{q_s} = 6 amplitudes (2 x 2^{q_s} = 4 in place) at q_s = 1, for (1,1) and (2,1) alike. Sum semantics (as (11.12')):
and_s = delta[x, z] eta[x'] + eta[x] delta[z, x']  ->  sigma = sigma_1 S_2 + S_1 sigma_2, S_i = sum_s sigma_i[s] (2^{q_s} + 2
complex numbers); a fully learned and_s is an ordinary word s^r s s^l with 2^{3 q_s} amplitudes (8 at q_s = 1 for both
(1,1) and (2,1)), the cost class of a post-verbal adverb. Circuit form (Sec. 11.5): both sub-sentences are STATE islands
(each survivor s is a state wire); the merge is mu = CNOT(s_1,r -> s_2,r), r = 0..q_s-1, postselect s_2 = |0..0> (amplitude
factor exactly 1, Sec. 11.3), the merged vector sits on s_1; W_hw at the merge step = W_1 + W_2 (both islands live), e.g.
"Alice loves Bob and Bob loves Alice" 3 + 3 = 6 qubits at (1,1), 5 + 5 = 10 at (2,1); the tensordot pays 2 + 2 + 2 = 6
amplitudes for the same step. Tree adjoint (Sec. 11.7): lambda_sigma_1 = conj(sigma_2) . lambda_sigma, lambda_sigma_2 =
conj(sigma_1) . lambda_sigma (verified on the shared-word sentence above, critic/adjtree.c, 1.5e-10 against central
differences).

Contraction order and peak for (11.14): t1[a, c] = sum_b V[a, c, b] Bob[b] (2^{2 q_n + q_s} complex MACs; live V, Bob,
t1); t2[a] = sum_c t1[a, c]; t2[a] *= Alice[a] (11.7); sigma[s] = sum_a t2[a] IV[a, s]. Peak amplitudes 2^{2 q_n + q_s}
+ 2^{q_n} + 2^{q_n + q_s} = 14 at (1,1), 44 at (2,1); with the Sec. 1.3 slot rule 10 and 36 -- identical to N TV N. A
relative clause costs no memory beyond the transitive verb it contains. (11.15) contracts Bob over the LOW field of V.
(11.16): 2^{q_n} + 2^{q_n + q_s} = 6 / 12; (11.17): 6 / 12 when sigma[s] accumulates the products word by word, 10 / 20
if both IVs are live. Flops (11.14): 8 + 4 + 2 + 4 = 18 complex ops at (1,1), 32 + 8 + 4 + 8 = 52 at (2,1). Frobenius
"and" is an elementwise product; a SUM semantics is

    and_n[a, b, c] = delta[a, b] eta[c] + eta[a] delta[b, c]   ->   sigma[s] = sum_b ( Alice[b] S_Bob + S_Alice Bob[b] ) IV[b, s],   (11.12')
    S_x = sum_i x[i]  (verb-side index b tied to Alice in the first term and to Bob in the second; verified to 3e-16)

and a fully learned "and" is an ordinary word n^r n n^l with 2^{3 q_n} amplitudes (8 / 64), the cost class of a
transitive verb.

### 11.4 Contraction order for arbitrary cup structures

Two views. (i) Bracket view: cups form a forest under nesting; D = maximum nesting depth. (ii) Word-graph view:
nodes = words, one edge per cup (spider legs included), root = the word owning the survivor. The word graph is a
tree for most sentences but NOT for all: every wrapper word of shape X s^l n (pre-verbal adverb, subject relative
pronoun, Lambek-form auxiliary or negation, and_vp) whose s^l reaches past k post-verbal modifiers closes a cycle of
length k + 2. Triangle with a unique parse: "Alice who sleeps quickly runs" = n | n^r n s^l n | n^r s | s^r s |
n^r s, cups (0,1) (4,5) (6,7) (3,8) (2,9): who-sleeps, sleeps-quickly, quickly-who. 4-cycle: "Alice quickly sleeps
in park happily" = n | n^r s s^l n | n^r s | s^r s n^l | n | s^r s, first parse (0,1) (3,12) (4,5) (6,7) (8,11)
(9,10): quickly-sleeps-in-happily-quickly. Over CFG-generated sentences of Sec. 11.2, 25-27 % contain a cycle
(refE/cyc3.c). Cycles are handled below; they never change the kernel, only the elimination order.

Bottom-up evaluation (tree part of the word graph). Post-order over the tree rooted at the survivor word: a leaf
is its 2^{Q_w} vector; an internal word contracts each child's reduced vector over the joining field(s) with the
fused loop of Sec. 1.3 (a spider node applies (11.7)/(11.9) instead of a MAC loop). The post-order HOLDS the
results of earlier siblings while a later subtree is evaluated, so the peak is a recurrence, not a per-word
maximum:

    peak(v) = max( max_k [ peak(child_k) + sum_{k' < k} 2^{q_{k'}} ],  2^{Q_v} + sum_k 2^{q_k} + 2^{q_out(v)} )       (11.18')
    children evaluated in DECREASING order of peak(child_k); q_k = total width of all cups joining v and child k;
    q_out(v) = width toward the parent (0 for the root: the survivor field is the output); drop the last term with
    the slot rule.  macs(v) = sum_k 2^{Q_v - sum_{k' < k} q_{k'}}  (each child pass touches the not-yet-contracted entries)  (11.19)

Example where the held-sibling term bites: "Alice who loves Bob sees Carol who hates Dan" (unique parse): while
hates is contracted with Dan, t_who1 (2^{q_n}) is held: 8 + 2 + 4 + 2 = 16 (12 in place) at (1,1), 32 + 4 + 8 + 4 =
48 (40) at (2,1), against 14 (10) / 44 (36) for N TV N. Any other order is worse. The extra term is always a few
2^{q} vectors: the widest WORD sets the budget, not the sentence,

    peak_sentence <= 2^{Q_max} + (number of words) x 2^{q_max}  (crude), typically 2^{Q_max} + 2..4 x 2^{q}          (11.20)
    fp32 complex: 8 B per amplitude; fp16 / Q1.15: 4 B.

The procedure, diagram -> contraction (C16 data; this is the whole of what criticism (1) needs beyond the parser).
Inputs: nw words with type strings (word[p], factor[p], width(p) for every global type position p, C7), cups[] from
Sec. 11.1, survivor. Outputs: tree[] (Node, post-order) consumed by eval_tree. Nothing below depends on planarity.

    // ---- build_tree: cups[] + survivor -> word graph -> spanning tree + chains -> post-order Node[] --------------------
    // Edge e(wp, wq) between DIFFERENT words; parallel cups (and_vp -- verb; who -- loves) merge into one edge:
    //   e.mask[wp] |= 1 << factor[p];  e.mask[wq] |= 1 << factor[q];  e.q += width(p)      (q_k of (11.18') = e.q)
    for each cup (p, q): wp = word[p]; wq = word[q];
        assert(wp != wq);                    // an intra-word cup cannot occur for the Sec. 11.2 lexicon (no type string of
                                             // the lexicon contains an adjacent t t^r or t^l t pair); reject the parse if it does
        e = find_or_add_edge(wp, wq); e.mask[wp] |= 1 << factor[p]; e.mask[wq] |= 1 << factor[q]; e.q += width(p);
    root = word[survivor];
    // DFS from root over the word graph (spider words are ordinary nodes here; colour is a Sec. 11.5 notion, not a
    // contraction notion). Tree edges = DFS tree; a BACK edge (v, u), u a proper ancestor of v, closes ONE cycle
    // u - ... - v - u along the DFS path; u is the WRAPPER (the word whose s^l / n reaches past the modifiers, Sec. 11.4).
    // The word graph of a planar cup set has at most one back edge per wrapper; nested cycles do not occur in the
    // Sec. 11.2 lexicon (assert: every edge is on at most one cycle; else fall back to the streaming form (11.21)).
    dfs(v): mark v; for each edge (v, x) not yet used: if x unmarked { parent(x) = v; dfs(x); } else if x != parent(v) &&
            x is an ancestor of v: record chain C = path(x -> v) as { wrapper = x, members = [first child of x on the
            path, ..., v], closing_edge = (v, x) }.
    // Chain members: each member m (in DFS order) keeps its DFS children that are NOT on the path as ordinary tree
    // children (absorbed first, in eval); its two open fields are e(m, prev).mask[m] and e(m, next).mask[m] (prev / next
    // along the chain; for the first member prev = wrapper via the tree edge, for the last member next = wrapper via
    // the closing edge). The wrapper sees the chain as ONE child with cmask = e(wrapper, first).mask[wrapper] |
    // e(last, wrapper).mask[wrapper] and q_k = e(wrapper, first).q + e(last, wrapper).q.
    // peak(v) by (11.18') bottom-up (a chain child costs 2^{q_a+q_b} + 2^{q_b+q_c} + the live factor, (11.18'')), then
    visit(v):  sort children(v) by DECREASING peak (ties: text order);  for k in children(v): visit(k)
               [for a chain child: visit its members in chain order (each with its own tree children first), then emit
                Node{ kind = 2, child[] = member node indices in chain order, nchild = #members }];
               emit Node{ word = v, nchild = #children (chain counted once), child[k] = node index of k,
                          cmask[k] = e(v, k).mask[v] (chain: the OR above),
                          pmask = (v == root) ? (1 << factor[survivor]) : e(v, parent(v)).mask[v],
                          kind = (ansatz_id(v) == 0xFE) ? 1 : 0 };
               assert(nchild <= 4);     // sizeof(Node) = 14 B, C16
    visit(root);  tree[] is now in post-order, root last.  Cost: O(ncups + nw) time, nw Nodes + ncups edges of scratch
    (an edge = 2 u8 masks + 1 u8 width + 2 u16 words = 7 B): a 20-word sentence needs about 280 B (tree) + 140 B (edges).

    // ---- eval_tree: R[v] = reduced vector of node v, 2^{q_out(v)} complex, in SMEM / VTCM; word tensor in registers -----
    // Field bookkeeping: a vector R[k] carries the list of fields it still holds (which factors of the OWNING word, C7
    // order, little-endian in list order as in Sec. 1.3); the cup pairing between a field of T and a field of R[k] is
    // fixed by the cups (component-wise bit r <-> bit r, C8; nested: bit-reverse the k-field).
    for v in tree[] (post-order):
      if (kind(v) == 0) {                                                 // unitary word: Sec. 1.3 fused loop per child
        T = generate(phi_v)  (C10, 2^{Q_v} amplitudes);  FT = all factor fields of v (offsets fo_v[i], C7);
        for k in children(v) (the sorted order):                           // child k plays the 'incoming word' of Sec. 1.3
            T = fused_tensordot(T, FT, R[child k], fields(R[child k]));    // cup fields: the cmask[k] bits of FT against
                                                                           //   ALL fields of R[child k] (they all join v)
            FT = FT minus the cmask[k] fields;  release R[child k];       // output fields: restT (low) then restW (high) = {} 
        R[v] = T;                                                          // remaining fields = pmask(v) (root: the survivor)
      } else if (kind(v) == 1) {                                          // spider (C15): one index per BASIC TYPE
        // For each basic type b present in v: n_b = #factors of type b.  OUT = { b : pmask(v) has a factor of type b }
        // (delta with the parent as one leg);  SUM = { b : no factor of b toward the parent }: n_b == 1 -> eps (11.9),
        // n_b >= 2 -> a closed delta = sum over the shared index.  Exactly one index i_b per type: THAT is the delta.
        for o in 0 .. 2^{sum_{b in OUT} q(b)} - 1:
          acc = 0
          for e in 0 .. 2^{sum_{b in SUM} q(b)} - 1:
            i_b = bit group of (o, e) for type b (OUT types low, in pmask order; SUM types high)
            prod = 1
            for k in children(v):                                         // every child's field of type b reads index i_b
                prod *= R[child k][ sum over fields f of R[child k]:  i_{type(f)} << shift_f ]
            acc += prod
          R[v][o] = acc                                                    // (11.7) products, (11.9) sums, fused
        // who   : OUT = {N} (n toward the parent), SUM = {S} (one s^l -> eps):  R[a] = Alice[a] sum_c R_loves[a + 2^{q_n} c]  = (11.14)
        // whom  : OUT = {N}, SUM = {S}: R[a] = Alice[a] sum_c R_loves[c + 2^{q_s} a]   (loves' open fields: s LOW, n^l HIGH, C7)
        // and_n : OUT = {N}, SUM = {}:   R[a] = Alice[a] Bob[a]                                                            = (11.16)
        // and_vp: OUT = {S}, SUM = {N}:  sigma[s] = sum_a Alice[a] IV1[a + 2^{q_n} s] IV2[a + 2^{q_n} s]                  = (11.17)
        // and_s : OUT = {S}, SUM = {}:   sigma[s] = sigma_1[s] sigma_2[s]                                                = (11.17')
        // cost: 2^{sum_b q(b)} x nchild complex multiplies (who: 2^{q_n + q_s} x 2 = 8 at (1,1));  memory: R[v] only.
      } else {                                                            // kind 2: chain (11.18''), members v_1 .. v_k
        // each member's own tree children were absorbed by its Node op above; R[v_i] has exactly two open fields:
        // toward v_{i-1} (LOW, row) and toward v_{i+1} (HIGH, col); view it as M_i[2^{q(v_{i-1}, v_i)}][2^{q(v_i, v_{i+1})}]
        M = R[v_1];  for i = 2 .. k: M = M x R[v_i]  (ordinary complex matmul; row index = field toward the wrapper side,
            C7 bit order inside each field);  peak 2^{q_a + q_b} + 2^{q_b + q_c} + the live factor;
        R[v] = M with fields { low = v_1's field toward the wrapper, high = v_k's field toward the wrapper };
        // the wrapper then contracts BOTH fields in one fused_tensordot pass (unitary wrapper, (11.18'')) or reads
        // M[i][i'] through the spider loop above (spider wrapper: costs nothing extra, the delta ties i = i').
      }
    sigma = R[root]   (2^{q_s} complex); Z, p as Sec. 1.5.

    // Triangle "Alice who sleeps quickly runs": chain [sleeps, quickly] under the spider wrapper who: M = IV[a][s] x ADV[s][s']
    // (2^{q_n} x 2^{q_s} times 2^{q_s} x 2^{q_s}); who: OUT = {N} (toward runs), SUM = {S} (s^l -> eps over s'):
    // R_who[a] = Alice[a] sum_{s'} M[a][s'].  4-cycle "Alice quickly sleeps in park happily": chain [sleeps, in(+park),
    // happily] under the unitary wrapper quickly (n^r s s^l n): M[a][s''] = IV x IN' x ADV, contracted into quickly's
    // factors 3 (n) and 2 (s^l); what remains, fields n^r and s, is then contracted with Alice: peak 16 + 4 + 4 = 24 at (1,1).

Worked example "Alice who loves Bob believes that Carol sleeps": positions Alice 0; who 1-4; loves 5-7; Bob 8;
believes 9-11; that 12-13; Carol 14; sleeps 15-16; cups (0,1) (4,5) (7,8) (3,6) (2,9) (11,12) (13,16) (14,15);
survivor 10. Root believes; children: the who-subtree (via n^r) and the that-subtree (via s^l). Post-order:
t_loves[a] = sum_c sum_b V[a,c,b] Bob[b] (peak 14 / 44); t_who[a] = Alice[a] t_loves[a]; t_sleeps[c] = sum_d
Carol[d] IV[d, c] (4 + 2 + 2); THAT = s s^l has factor 0 (s, LOW bits, position 12) toward believes and factor 1
(s^l, HIGH bits, position 13) toward sleeps, so

    t_that[c] = sum_{c'} THAT[c, c'] t_sleeps[c'],  THAT[c, c'] = phi_that[c + 2^{q_s} c']   (contract over the HIGH index)   (11.14')

(verified 1.8e-15 against brute force; the transposed form is wrong by O(1)). Then believes B[a, s, c] = phi[a +
2^{q_n} s + 2^{q_n + q_s} c]: sigma[s] = sum_a sum_c t_who[a] B[a, s, c] t_that[c]: 8 + 2 + 2 + 2 = 14 at (1,1),
16 + 4 + 2 + 2 = 24 at (2,1). Whole sentence: 14 (10) / 44 (36), dominated by loves, provided the who-subtree is
evaluated first (its peak is larger).

Cycles. Contract a cycle as a CHAIN of matrix products over the s (or n) wires, starting at the word opposite the
wrapper and ending at the wrapper, holding two open fields; through a SPIDER wrapper (who) the cycle costs
nothing extra because the spider is (11.7). For a unitary wrapper of width Q_wrapper and open fields q_a, q_b:

    peak_cycle <= 2^{Q_wrapper} + 2^{q_a + q_b} + 2^{Q_wrapper - q_a - q_b}    (4-cycle above at (1,1): wrapper n^r s s^l n:
                  16 + 4 + 4 = 24;  at (2,1): 64 + 8 + 8 = 80)                                                          (11.18'')

Streaming (left-to-right) evaluation and the cut-width bound. If words must be consumed in sentence order, the open
tensor T at boundary x holds one field per cup (i, j) with i < x <= j plus the survivor if already seen. Cups
crossing one boundary are pairwise nested (planarity), so their number is at most D + 1 and

    cw(x) <= D + 1,   |T| <= 2^{q_max (D+1)},   peak_stream <= 3 x 2^{max(q (D+1), Q_max)},
    log2(peak_stream) <= max(q (D+1), Q_max) + 2  (+1 with the slot rule)                                              (11.21)

For the worked sentence the boundary before "loves" has open cups (2,9) (3,6) (4,5): cw = 3, |T| = 2^{2 q_n + q_s} =
8 / 32; loves 8 / 32; R over the open fields {2, 7} = 2^{2 q_n} = 4 / 16: peak 20 / 80 versus 14 / 44 bottom-up.
Recommendation: parse the whole sentence, build tree[] (C16), evaluate by (11.18'); the stream form is for a
device that cannot hold the m type records (6 B each), which never happens.

### 11.5 Bent-wire circuit form for arbitrary cup structures

Rule (Sec. 1.4): a unitary word is an EFFECT (apply U_w^T per C11 on its partners' qubits, postselect |0..0>) iff
every one of its factors cups to a STATE word's wire; the survivor word is a state; two state words joined by a
cup need the Bell gather (2.6) with its 2^{-q/2}; two effect words are never joined. Spider words have NO colour:

    colour(root) = STATE;  BFS over UNITARY words with spiders transparent: colour(child) = !colour(parent);
    a spider's legs to STATE words are merged on the live wires (CNOT ladder + postselect, factor 1); of its legs to
    non-state words one is an EFFECT applied on the merged wire; each FURTHER effect leg needs Delta onto a fresh
    q-qubit |0> ancilla (CNOT ladder) before U^T + postselect; eps legs = H on each qubit + postselect (2^{-q/2});
    an odd cycle of unitary words (they occur: post-verbal modifiers under a relative pronoun) marks one cup "Bell" (2.6)  (11.22)
    wire_map for an effect word: local qubit fo_w[j] + r -> start(partner position of factor j) + r  (C8; nested: start + q - 1 - r)

    W_hw = max over steps of live qubits = max_w [ Q_w(state) + sum over child islands still held q_c ] + q x (effect legs - 1)
           summed over spider nodes whose ancillas are live at that step                                                (11.23)

Example (11.14): states {loves, sleeps}, effects {Alice, Bob}, who transparent. loves online (2 q_n + q_s); Bob^T on
n^l, postselect; H + postselect on s (eps); sleeps Kronecker-in (2.7); CNOT(sleeps.n^r -> loves.n^r), postselect
loves.n^r = 0 (merge; the merged vector sits on sleeps.n^r); Alice^T on sleeps.n^r, postselect. W_hw = 2 q_n + q_s =
3 (8 amplitudes) at (1,1), 5 (32) at (2,1); final amplitude = sigma[s] 2^{-q_s/2}, p_surv = Z 2^{-q_s} (simulated
verbatim, verE/frob.c). "Alice and Bob sleep": legs Alice and Bob are both effects: W_hw = 2 q_n + q_s (one Delta
ancilla), not q_n + q_s. Sentential complement "Alice believes that Bob loves Carol": island-first W_live = q_n + 3 q_s
= 4 (16) at (1,1), 5 (32) at (2,1); left-to-right 3 q_n + 3 q_s = 6 / 9 (64 / 512) -- the "early-cup monolithic" of
Sec. 2.3, which is what a naive circuit compiler emits. The circuit form is worse than the tensordot whenever two
multi-wire state words are simultaneously live, by 2^{q x (open fields)}.

Training memory in the CIRCUIT form. Sec. 5.3's adjoint recurrence inverts unitaries only ("only UNITARIES are in the
gate list"); the island-first schedule above postselects and compacts MID-circuit, which the recurrence cannot invert.
If training is done on the circuit at all, it runs the UNCOMPACTED circuit, W_train = sum over STATE words of Q_w, with
every postselection inside O_c as in Sec. 5.3, adjoint unchanged:

    (11.14): W_train = (2 q_n + q_s) + (q_n + q_s) = 5 / 8  ->  24 x 2^W = 768 B / 6 KB
    sentential complement (believes + loves): 6 / 9  ->  1.5 KB / 12 KB                                                    (11.23')

Growth law of (11.23'). The 2-colouring (11.22) makes about half of the unitary words states, so

    W_train = sum_{state words} Q_w  ~  (n_unitary / 2) x mean Q_w;   memory 24 x 2^{W_train} B (fp32, 3 vectors)         (11.23'')
    "Alice believes that Bob thinks that Carol sleeps": states {believes, thinks, sleeps}: W_train = 3 + 3 + 2 = 8 at (1,1),
        4 + 4 + 3 = 11 at (2,1)  ->  6 KB / 48 KB;
    a 20-word sentence at (2,1): W_train ~ 25-30  ->  24 x 2^25 .. 24 x 2^30 = 0.75-24 GB;
    ceiling: 24 x 2^W <= 48 KB SMEM -> W <= 11;  <= 8 KB VTCM slice -> W <= 8;  <= 1 KB registers -> W <= 5,
    i.e. the circuit form trains at most a clause pair on-chip and NOTHING of Sec. 11 length in registers.

This is why the circuit form is NOT the training path: Sec. 11.7 gives the exact reverse mode on the word tree, whose
memory is independent of the sentence (400 B for the T12 sentence, 1.7 KB for 20 words at (2,1)). The circuit form
(11.23') is kept only for hardware and for shapes with <= 2 state words, where it is the same size. (The mid-circuit
alternative -- compaction^dag = embed with zeros at the postselected slots; kron_in^dag = contract lambda' against
conj(phi_w) over the word's field, then back-propagate through U_w's own gate list; one checkpoint of the pre-projection
state per postselection -- is subsumed by 11.7 and not specified further.)

### 11.6 Where the ceiling is

Formal power. Pregroup grammars are weakly equivalent to context-free grammars (Buszkowski 2001, UNVERIFIED year), so
cross-serial dependencies (Dutch "dat Jan Piet Marie zag helpen zwemmen": NP_i bound to V_i; Swiss German case
marking) have NO planar type assignment. Options: (a) accept a pregroup parse with the wrong dependencies; (b) drop
planarity in the parser only -- the kernel (11.18') never used it; a crossing cup set is still a word graph.
Recognition of crossing matchings is polynomial: admissible pairs join opposite z-parities, so the compatibility
graph is bipartite and a perfect matching is decided by Hopcroft-Karp in O(m^2.5) (counting them is #P-hard;
ambiguity, not recognition, is the expensive part). (c) In practice: take the cup set from a classical dependency
parser (an arc labelled (b, z)-(b, z+1) is a cup; non-projective arcs are fine) and keep the pregroup machinery for
well-typedness. The model is then "tensor contraction over a dependency tree", which is what Sec. 1.3 computes.

Ellipsis ("Alice loves Bob and Carol does too") needs the verb tensor twice: on the simulator (11.8), a second read of
phi_V (0 B); on hardware a basis copy (CNOT), never a state copy. Anaphora and text: pregroups end at the sentence;
DisCoCirc (Sec. 14) keeps one wire per entity at 2^{q_n E_live} amplitudes: at 400 B of state E_live <= 5 (q_n = 1) or
2 (q_n = 2); at 4 KB (HVX file, fp16 / Q1.15) or 8 KB (VTCM slice, fp32) 10 / 5; at 100 KB SMEM 13 / 6. (1, E = 8) =
256 amplitudes = 2 KB: CUDA Layout B warp registers, HVX 16 vectors ("tight", Sec. 3.2) or VTCM; a single CUDA thread
cannot hold it. Beyond that: MPS truncation (Sec. 14.3).

Ambiguity. Several parses p with weights w_p >= 0 (parser scores, 1/|P|, or learned):

    sigma[s] = sum_p w_p sigma_p[s]              (superposition; one extra 2^{q_s} accumulator, |P| x the MACs)          (11.25)
    rho      = sum_p w_p |sigma_p><sigma_p| / Z_p   (mixture; 2^{2 q_s} accumulator, readout rho[k][k])                   (11.26)

Both exact, 2 or 4 extra complex numbers at q_s = 1. (11.25) is not normalised per parse; use w_p / sqrt(Z_p) if parses
should count equally.

Robustness on real text. Every word must map to a type string and an angle block. Unknown words: a POS tagger written
in C++/Mojo (averaged perceptron with hashed features: ~50 lines of logic, 0.5-4 MB of fp16 weights, UNVERIFIED
accuracy; it dwarfs the 100 KB model and appears in the Sec. 16 budget), a fixed tag -> type table (NN, NNP -> n;
JJ, DT -> n n^l; VBZ / VBD intransitive vs transitive by a subcategorisation table defaulting to n^r s n^l when the
next token is nominal; IN -> n^r n n^l or s^r s n^l by the previous tag; WP -> who / whom by case; CC by the
neighbouring tags), and OOV angles from a per-tag shared block or by feature hashing (Sec. 13.1). Interface:

    struct Word { uint32_t lex_id; uint8_t ntypes; TypeRec types[6]; uint32_t angle_off; };   // 48 B
    int  front_end(const char* utf8, int len, Word out[], int max_words);                 // tokenise, tag, lemmatise, look up
    int  parse(const Word* w, int nw, Cup cups[], int* survivor, uint32_t* nparses, int8_t target);   // chart (11.4)-(11.5)
    int  build_tree(const Cup* cups, int ncups, int survivor, int nw, Node tree[]);       // C16 word graph, post-order, cycles
    // the node-op kernel (C16) consumes tree[] and angle_off; spider words carry ansatz_id 0xFE and no angles

Failure policy (fragments, punctuation, dialogue) is the front end's: drop; back off to the longest parsable prefix,
accept_prefix(j) = OR_p [ types[p] == (S,0) && inner(0, p-1) && inner(p+1, j) ]; or split at conjunctions.

Summary. Grammar: everything context-free and planar is covered by 11.1-11.2 with zero extra kernel code (about 25
node-op instantiations, C16); crossing dependencies need a non-pregroup parser but not a different kernel; text-level
phenomena need DisCoCirc and pay 2^{q_n E_live}. Memory: for any sentence, however long or deeply nested, the exact
contraction peak is (11.18') = 2^{Q_max} + a few 2^{q} vectors, and the exact training memory is (11.31) = that peak
plus one 2^{q_out} vector per word plus 3 x 2^{Q_max}; only the circuit form pays (11.23'') and grows with nesting.

### 11.7 Reverse-mode training on the word tree (the training path; the circuit of 11.5 only for <= 2 state words)

Sec. 5.3 differentiates a CIRCUIT: one register, unitaries only, three vectors of 2^{W_train}. Sec. 11 sentences are
evaluated as a TREE of node ops (11.18'), and the node op is complex-MULTILINEAR in its operands (the word tensor and
every child's reduced vector), so its adjoint is one node op per operand with the roles swapped. Nothing else is
needed: the postselection never appears (cups are contractions, Sec. 1.2; Z enters only through the root weights).

Cotangent convention (that of (5.8), so Sec. 5.3's recurrence is reused verbatim):

    lambda_x[j] = (1/2) ( dL/dRe x[j] + i dL/dIm x[j] ),  hence  dL = 2 Re sum_j conj(lambda_x[j]) dx[j];
    for any complex-linear y = A x:  lambda_x = A^H lambda_y   (A^H = conjugate transpose)                                   (11.27)

Node-op forward (Sec. 1.3, unitary word v with children k, word tensor phi_v of 2^{Q_v} entries, index j; out(j) and
f_k(j) = the bit fields of j toward the parent and toward child k, C7):

    r_v[out] = sum_{j : out(j) = out}  phi_v[j] prod_k r_k[ f_k(j) ]                                                        (forward)

which is complex-linear in phi_v and in every r_k separately; therefore

    root:     lambda_sigma[c] = w_c sigma[c],  w_c = (g_c - gbar) / D,  g_c = dL/dP_c   (= (5.8) verbatim);
              for L = -ln p_y:  w_c = (1 - delta_{c y} / p_y) / Z                                                           (11.27')
    word:     lambda_phi_v[j]  = lambda_{r_v}[ out(j) ] * conj( prod_k r_k[ f_k(j) ] )         2^{Q_v} complex multiplies   (11.28)
    child k:  lambda_{r_k}[x]  = sum_{j : f_k(j) = x}  conj( phi_v[j] prod_{k' != k} r_{k'}[ f_{k'}(j) ] ) * lambda_{r_v}[ out(j) ]
                                                                                              2^{Q_v} complex MACs          (11.29)
              (= the Sec. 1.3 fused loop with the OUTPUT field replaced by the cotangent input, child k's field left
               open, every other operand conjugated; the same node-op instantiation with a conj flag and one field
               role permuted)
    spider:   mu (11.7):  lambda_u = conj(v) . lambda_out,  lambda_v = conj(u) . lambda_out;
              eps (11.9): lambda_u[i] = lambda_out (broadcast);  general spider loop of Sec. 11.4: the cotangent of
              child k at its index = sum over the SUM indices of conj(product of the other children) x lambda_out[o];
              Delta / second read of the same tensor (ellipsis, Sec. 11.6) and shared angles (two occurrences of a
              word, Sec. 5.5): cotangents / gradients ADD.
    chain:    every matrix product of (11.18'') is bilinear: lambda_{M_i} = (M_1 .. M_{i-1})^H lambda_M (M_{i+1} .. M_k)^H,
              formed right-to-left with one matmul per factor; then (11.28)-(11.29) on each member.
    angles:   Sec. 5.3 recurrence on the WORD circuit alone (width Q_v): A = phi_v (regenerated), B = lambda_phi_v;
              for k = Ng_v down to 1: C = G_k A; grad_local[k] = Im( sum_b conj(B[b]) C[b] ); A = U_k^dag A; B = U_k^dag B    (11.30)
              sign_k = +1 for EVERY gate: the tree never transposes a word (the C11 / Sec. 5.3 sign rule belongs to the
              circuit form only); dL/dtheta_w[i] += grad_local (scatter-add over occurrences, Sec. 5.5).

Order per node, in REVERSE post-order (root first): (a) regenerate phi_v from its angles (or keep it from the forward pass
if the budget allows); (b) for each child k: (11.29) -> lambda_{r_k} (2^{q_k} complex, stored until that child is visited);
(c) (11.28), overwriting phi_v with lambda_phi_v; (d) regenerate A = phi_v and run (11.30). Every forward reduced vector
r_v is kept from the forward pass (it is an operand of its siblings' (11.29) and of its parent's (11.28)).

    M_train = sum_w 2^{q_out(w)}  +  3 x 2^{Q_max}  +  peak(11.18')     amplitudes (fp32 complex, 8 B each; fp32 ONLY, Sec. 5.3)  (11.31)
              (root: q_out = q_s, its r is sigma;  the 3 x 2^{Q_max} are A, B, C of (11.30) at the widest word)
    T12 sentence "Alice who loves Bob sleeps" at (1,1): (2 + 2 + 4 + 2 + 2) + 3 x 8 + 14 = 50 amplitudes = 400 B;
    a 20-word sentence at (2,1) (mean 2^{q_out} = 4, Q_max = 5 (TV), peak 44): 80 + 96 + 44 = 220 amplitudes = 1.7 KB;
    versus 24 x 2^{W_train} = 0.75-24 GB for the uncompacted circuit (11.23'').  Tier: REGISTER / VTCM for every sentence
    of Sec. 11; the sentence length enters only linearly through sum_w 2^{q_out(w)} (about 4-8 B x q-fields per word).
    Flops: (11.29) once per child (each 2^{Q_v} MACs, i.e. one forward node op per child), (11.28) 2^{Q_v} multiplies,
    (11.30) about 3 word generations (regenerate, U^dag sweep on A and on B) plus one generator application per gate:
    total ~3-4x the forward pass, exactly as Sec. 5.3. Round-off O(Ng_max u32) per word, not O(Ng_total): the tree
    adjoint is BETTER conditioned than the circuit adjoint on long sentences.

Verified (critic/adjtree.c, fp64, central differences h = 1e-5): "Alice loves Bob" (root loves; children Alice, Bob)
reproduces the 8 dLoss(y=0)/dtheta values of Sec. 8 (-0.1204452, -0.0784477, 0.0712521, -0.3122115, 0.1854557, -0.1553682,
0.3558636, 0.9490759) with |adjoint - central| <= 4.8e-11; the T12 sentence (9 angles; spider who with mu / eps adjoints):
dL/dtheta = [-0.15106071, -0.14060757, 0.17728899, 0.04493042, -0.01820026, 0.02841004, 0.03290603, -0.10412078,
0.61601074] (Alice t0 t1 t2, Bob t0 t1 t2, loves t0 t1, sleeps t0), max deviation 3.1e-11; "Alice sleeps and dreams"
(and_vp spider, 5 angles): [-0.03859959, -0.02433568, 0.02115842, 0.43442175, 0.43442175], 1.3e-11 (the last two are equal
because IV1 . IV2 depends on t_sleeps + t_dreams only under IQP L = 1); "[Alice loves Bob] and_s [Bob loves Alice]" with
SHARED angles (8 angles, each word occurring twice): [-0.02308977, 0.00129679, -0.02036071, -0.08223878, 0.01134526,
-0.06765975, 1.06704471, 1.84830706], 1.5e-10 -- the scatter-ADD over occurrences is what makes this correct.

What this does not change: the circuit form is still the HARDWARE form and the only one with a shift rule (Sec. 5.2);
the tree adjoint is a simulator-only training rule. It also does not change the CAPACITY ceiling of Sec. 12: it makes
the training memory sentence-independent, not the model wider.

---------------------------------------------------------------------------------------------------------------

## 12. Capacity and scaling laws

d = 2^{q_n}, d_s = 2^{q_s}, Q = 2 q_n + q_s (transitive-verb width). Sec. 8's sigma, Z, p, the (12.10) entries and the
L_sat table were checked with an fp64 C reference (derivF/sym.c, vfy.c); the remaining tables are integer
arithmetic on (12.11).

### 12.1 Dimension counting

A pure state on Q qubits modulo global phase and norm is a point of CP^{2^Q - 1}:

    dim_R(pure states, Q qubits) = 2 (2^Q - 1) = 2^{Q+1} - 2;   dim_R(REAL pure states) = 2^Q - 1                  (12.1), (12.2)

Sentence: 2^{q_s+1} - 2 real dof (q_s = 1: 2, the Bloch sphere; 2: 6; 3: 14). Noun: 2, 6, 14, 30 for q_n = 1..4.
Verb (Q = 2 q_n + q_s): 14 (Q = 3), 62 (Q = 5), 2046 (Q = 10). Parameters per word (C10) and the depth at which the
count alone could cover the manifold (necessary, not sufficient; sufficiency for Sim14/15 at L_sat is UNVERIFIED in
general, but see the rank checks below):

    IQP (0/1): P_w = L (Q-1)        L_sat >= (2^{Q+1} - 2) / (Q-1)
    Sim15:     P_w = 2 Q L, REAL    L_sat >= (2^Q - 1) / (2 Q)       (reaches only (12.2), never a complex state)
    Sim14:     P_w = 4 Q L          L_sat >= (2^{Q+1} - 2) / (4 Q)
    Ry-only:   P_w = Q, REAL        never saturates for Q >= 2                                                       (12.3)

    Q      2    3    4    5    6    7    8    9   10   11   12   13   14   16   18
    Sim15  1    2    2    4    6   10   16   29   52   94  171  316  586 2048 7282   (real manifold (12.2))
    Sim14  1    2    2    4    6   10   16   29   52   94  171  316  586 2048 7282   (complex manifold (12.1))
    IQP    6    7   10   16   26   43   73  128  228  410  745 1366 2521 8738 30841

Sim14 and Sim15 coincide because 2 Q L real angles cover 2^Q - 1 real dof exactly when 4 Q L angles cover 2^{Q+1} - 2
complex dof. At saturation every ansatz has P_w ~ dof (to one layer's rounding); what the IQP row shows is that IQP
needs about 4x more LAYERS (gates, flops, depth) for the same parameter count, i.e. it is the least layer-efficient;
Ry-only never saturates and is the least expressive.

Q = 1 words: Rx Rz Rx has 3 angles for a 2-dof manifold; the map (t0, t1, t2) -> U|0> has a rank-2 Jacobian, i.e. a
one-dimensional redundant fibre (not aligned with any single angle; there is no Rz acting on |0> in the C4 order).
Ry-only has 1 angle for a 1-dof real circle. Numerical projective-Jacobian ranks at a random point (fp64 central
differences): noun Rx Rz Rx rank 2 (full); Sim15 Q = 2 L = 1 (4 angles) rank 3 = full real dof; Sim14 Q = 2 L = 1
(8 angles) rank 6 = full complex dof; Sim14 Q = 3 L = 2 (24 angles) rank 14 = full; Sim15 Q = 3 L = 2 rank 7 = full
real; IQP Q = 2 L = 1 rank 1 of 6; IQP Q = 3 L = 1 rank 2 of 14; Sim15 Q = 3 L = 1 rank 6 of 7. So under Sim14 / Sim15
every Q <= 3 word saturates its manifold (locally) with <= 24 angles: at (1,1) the ansatz is NOT the bottleneck and
criticism (2) is about d = 2, not about the angle count. The "handful of angles saturates only 1-qubit nouns"
statement is true for IQP alone.

IQP, ansatz 0, L = 1: every amplitude has modulus 2^{-Q/2} (C10) and the phases are the LINEAR form

    arg V[k] = sum_{j=0}^{Q-2} b_j (2 b_{j+1} - 1) theta_j / 2                                                        (12.4)

(verified at Q = 4 to 1.2e-16; reproduces Sec. 8's eight phases), a (Q-1)-torus inside the (2^Q - 1)-dimensional phase
torus, a measure-zero subset of CP^{2^Q - 1}: it never reaches an arbitrary state at L = 1. Independent real functions
of one word's angles that the sentence can realise through that word:

    r_w = rank( P_perp d phi_w / d theta_w ) <= min( P_w, 2^{Q_w+1} - 2 )      (P_perp projects off span_R{phi_w, i phi_w})   (12.5)

Analytical diagnostic only; if needed offline: J (2 x 2^Q rows x P_w columns) by central differences h = 1e-4 in fp64,
project the columns, r_w = number of singular values > 1e-8 s_max by the one-sided Jacobi SVD of Sec. 14.3. Depth
fixes IQP's thinness, but L_sat ~ 2^Q / Q is exponential, at which point P_w ~ 2^{Q+1} and the angle table costs as
many bytes as the amplitudes (2 B per angle vs 2 B per real fp16 amplitude, 4 B complex); storing the vector directly
(Sec. 1.6) is then cheaper and gate-free. Angle parameterisation saves memory exactly when it is expressively thin.
That is the first ceiling.

### 12.2 The function class

Every gate entry is a degree-1 trigonometric polynomial in its half-angle (C5, C13). For a sentence WITHOUT repeated
word ids each parameter occurs in one gate on the path, each amplitude of phi_w is a product with one factor per
gate, and (1.3) is multilinear in the word states:

    sigma[s](theta) = sum_omega c_omega(s) exp(i omega . theta / 2),  omega_k in {-1, +1} for 1q rotations, {-1, 0, +1} for CRz/CRx  (12.6)
    N_c = |sigma[c]|^2: omega_k in {-2, 0, +2} (1q; {-1, 0, +1} in theta) and {-2..+2} (controlled; {-1, -1/2, 0, 1/2, 1} in theta,
          4 pi-periodic, Sec. 5.6);   p_c = N_c / D, D = sum_k N_k: a RATIONAL trigonometric function                  (12.7), (12.8)
    term counts: sigma <= 2^{G_1q} 3^{G_ctrl};  N_c <= 3^{G_1q} 5^{G_ctrl}  (G = parameterised gates on the path)

A word repeated m times (determiners "the ... the ...", "Alice loves Alice") enters sigma with degree m in ITS
angles (omega up to +-m in theta/2, N_c up to +-2m): the two-term shift rule (5.2) fails on that parameter and
gradients are sums over occurrences (Sec. 5.2 / 5.5, adjoint unaffected). Multilinearity in the word STATE slots
phi_w (as separate tensor inputs, one per OCCURRENCE) is unconditional:

    sigma[s] = sum_{a,b} n1[a] V[a, s, b] n2[b]:  V(., .): C^d x C^d -> C^{d_s} BILINEAR;  <c|V(n1, n2) trilinear;  p_c = |<c|V(n1,n2)|^2 / Z   (12.9)

Comparison. (a) Classical tensor DisCoCat (Grefenstette & Sadrzadeh 2011; Kartsaklis et al.; d ~ 300-2000 count
vectors, verbs d x d or d^3, UNVERIFIED exact figures): the same bilinear map with real entries and no Z; ours is
that model with d = 2^{q_n}, complex unit-norm words and V restricted to a P_w-parameter family. (b) A transformer:
x_i' = sum_j softmax_j(x_i^T W_Q W_K^T x_j / sqrt(d_k)) W_V x_j -- the MIXING WEIGHTS depend on the inputs; the map
is not multilinear, is composed with MLPs 12-96 times, 1e8-1e11 parameters, d_model 768-12288 (UNVERIFIED figures).

What the compositional tensor model represents that a bag of words cannot: word order and argument structure.
With V_s[a][b] = V[a, s, b],

    sigma_ALB[s] - sigma_BLA[s] = A^T (V_s - V_s^T) B                                                                 (12.10)

zero for every bag model, zero in the tensor model for ALL (A, B) iff V_s is symmetric for every s (a subspace of
dimension d_s d(d+1)/2 out of d_s d^2, measure zero); for a FIXED pair it is d_s complex conditions, so order
sensitivity is generic, not universal. Sec. 8: V[0,0,1] - V[1,0,0] = 0.02790915 + 0.13768018 i, V[0,1,1] -
V[1,1,0] = 0.06734991 - 0.33224752 i; "Alice loves Bob" p_0 = 0.58263882, Z = 0.14026553; "Bob loves Alice" sigma =
[-0.01927012 - 0.25903817 i, 0.09018148 - 0.29128266 i], Z = 0.16045040, p_0 = 0.42051694.

What it cannot represent (properties of the class (12.9), independent of q_n, q_s, L): (i) context-dependent word
meaning -- phi_w depends on theta_w only, so a word is a fixed vector linearly filtered by its neighbours; polysemy
can be a MIXTURE (Sec. 13.2(c)) but resolution stays linear; (ii) information flows only along cups -- whatever the
grammar does not connect is invisible; (iii) world knowledge beyond V P_w angles and the type table; (iv) any
non-multilinear function of the word states (an if-then on word identity). Sec. 15.2 quantifies (i) and (iv).

### 12.3 Scaling laws for memory, precision and flops

Working set of the fused tensordot for N TV N (Sec. 1.3; WS_full always includes the second noun):

    WS_full = 2^Q + 2^{q_n + q_s} + 2 x 2^{q_n};   WS_inplace = 2^Q + 2^{q_n};   bytes = 8 WS (fp32 complex), 4 WS (fp16 / Q1.15),
    halved again for a REAL model (Sim15 with every Q = 1 word under ansatz 4; Sim15's Q = 1 words are complex by C10)   (12.11)

    q_n q_s  Q   d   verb amps  verb fp32  fp16/Q15  Sim15 P_w   dim 2^{Q+1}-2   WS_inplace  WS_full   E[Z] = 2^{-2q_n}  error x 2^{q_n+1}
     1   1   3    2        8      64 B      32 B       6 L            14             10        16         0.25 (0.243 MC)    x4
     1   2   4    2       16     128 B      64 B       8 L            30             18        28         0.25              x4
     1   3   5    2       32     256 B     128 B      10 L            62             34        52         0.25              x4
     2   1   5    4       32     256 B     128 B      10 L            62             36        48         0.0625 (0.061)    x8
     2   2   6    4       64     512 B     256 B      12 L           126             68        88         0.0625            x8
     2   3   7    4      128       1 KB    512 B      14 L           254            132       168         0.0625            x8
     3   1   7    8      128       1 KB    512 B      14 L           254            136       160         2^-6              x16
     3   2   8    8      256       2 KB      1 KB     16 L           510            264       304         2^-6              x16
     3   3   9    8      512       4 KB      2 KB     18 L          1022            520       592         2^-6              x16
     4   1   9   16      512       4 KB      2 KB     18 L          1022            528       576         2^-8              x32
     4   2  10   16     1024       8 KB      4 KB     20 L          2046           1040      1120         2^-8 (0.0039)     x32
     4   3  11   16     2048      16 KB      8 KB     22 L          4094           2064      2208         2^-8              x32
     5   1  11   32     2048      16 KB      8 KB     22 L          4094           2080      2176         2^-10             x64
     5   2  12   32     4096      32 KB     16 KB     24 L          8190           4128      4288         2^-10             x64
     5   3  13   32     8192      64 KB     32 KB     26 L         16382           8224      8512         2^-10             x64
     6   1  13   64     8192      64 KB     32 KB     26 L         16382           8256      8448         2^-12             x128
     6   2  14   64    16384     128 KB     64 KB     28 L         32766          16448     16768         2^-12             x128
     6   3  15   64    32768     256 KB    128 KB     30 L         65534          32832     33408         2^-12             x128
     7   1  15  128    32768     256 KB    128 KB     30 L         65534          32896     33280         2^-14             x256
     7   2  16  128    65536     512 KB    256 KB     32 L        131070          65664     66304         2^-14             x256
     7   3  17  128   131072       1 MB    512 KB     34 L        262142         131200    132352         2^-14             x256
     8   1  17  256   131072       1 MB    512 KB     34 L        262142         131328    132096         2^-16             x512
     8   2  18  256   262144       2 MB      1 MB     36 L        524286         262400    263680         2^-16             x512
     8   3  19  256   524288       4 MB      2 MB     38 L       1048574         524544    526848         2^-16             x512

Precision column (a CORRECTION to Sec. 1.2, which says E[Z] = 1). For Haar-random words the expected normalisation
is E[Z] = 2^{-2 q_n} (Monte Carlo, derivF/zdense.c, 400 triples: 0.243, 0.061, 0.0039 at (1,1), (2,1), (4,2); Sec.
8's Z = 0.14 is consistent with 1/4, not 1). Since Z = p_surv, the spec's bound (4.7) multiplies every readout
error by 2 / sqrt(Z) = 2^{q_n + 1}. Consequences: fp16-store error 4.9e-4 x 2^{q_n+1} = 1.6e-2 at q_n = 4, 0.25 at
q_n = 8; Q1.15 at W = 10: 7.8e-4 x 32 = 2.5e-2 at (4,2). By Sec. 4.5 rule (3) with tol = 0.05, p_min = (4 x 5e-4 /
0.05)^2 = 1.6e-3 = 2^{-9.3}: fp16 / Q1.15 STORAGE is generically valid for q_n <= 4 only; the "fp16 adds one to each
Q" statements below are byte statements, and rows q_n >= 5 in fp16 / Q1.15 are precision-infeasible unless training
drives Z up (which it may: the model is trained, not Haar-random; validate per Sec. 4.6).

Parameters vs dimension: at Q = 10 Sim15 L = 2 gives 40 angles for the 1023-dof REAL manifold (3.9 %; 2 % of the
2046-dof complex manifold that Sim15 cannot reach at all); L = 52 fills the real manifold (P_w = 1040 angles = 2080 B
vs 1024 real fp16 amplitudes = 2048 B). Sim14 at L = 52: 2080 angles = 4160 B vs 4096 B of complex fp16 amplitudes.

Residency (Sec. 3.1, 3.2, 3.4; ptxas-verified targets per C14). CUDA thread-per-sentence (Layout A): about 110
fp32 complex amplitudes after temporaries -> WS_inplace <= 110 -> Q <= 6: (2,2) (68 complex = 136 registers) is the
largest; (3,1) complex (272) does not fit; a REAL model admits (3,1) / (2,3) at 255. At 1024 threads/block only (1,1).
HVX sentence-per-lane Q1.15 (64 sentences per vector pair): WS <= 13 -> (1,1) only (10 in place). HVX amplitude-
per-lane: 2 ceil(WS/64) + temporaries <= 32: (4,1) (528 amplitudes, 18 vectors) and (3,3) are byte-feasible but
MARGINAL (UNVERIFIED: widening int16 products land in int32 register pairs, in-vector gates need permute and
lane-coefficient vectors, 18 + 8 + 2 + 4 = 32-34); the safe register figure is (3,2), with (4,1) streamed from
VTCM at no throughput loss once latency is hidden (Sec. 3.2). VTCM 8 KB slice, Q1.15: 2048 complex -> Q <= 10:
(4,2) fits (4160 B), (5,1) does not (8320 B). SMEM fp32: 48 KB -> 6144 complex -> Q <= 12 ((5,2) = 33,024 B = 32.3
KB); 99 KB -> Q <= 13; 227 KB -> Q <= 14 ((6,2) = 131,584 B = 128.5 KB); fp16 storage adds one Q in bytes (see the
precision caveat). Adreno (~16 fp32 registers per fibre, UNVERIFIED): not (1,1) in fp32 registers at full
occupancy; (1,1) fits as packed fp16 in place (10 half2 registers, Sec. 3.4); larger shapes use local memory
(32 KB -> Q <= 11 fp32).

Flops per sentence. Sim15 on the verb: 2 Q L Ry gates + 2 Q L CNOTs per word (C10); a real Ry butterfly is 4
FMA-class per pair on 2^{Q-1} pairs = 2^{Q+1} per gate; CNOT is a compile-time renaming (Sec. 3.1) or 2^{Q-1} swaps:

    generation ~ 4 Q L 2^Q FMA-class (real Sim15) = 82 K at Q = 10, L = 2;  ~16 Q L 2^Q for a complex ansatz of equal gate count
    contraction = 2^Q + 2^{q_n + q_s} complex MACs = 1088 complex MACs = 4.4 K FMA-class at Q = 10                       (12.11')

Generation, not contraction, is the compute cost of the angle model; the vector model (Sec. 1.6) pays only the
contraction.

### 12.4 The parameter-count ceiling

    Lexicon bytes = 2 V mean(P_w)  (uint16 half-angle turns, C5):  V = 1e4, P_w = 5: 100 KB (Sec. 7);
    V = 5e4, P_w = 24: 2.4 MB;   training state (Sec. 5.5) 16 B/param: 19.2 MB for the same lexicon                     (12.12)

2.4 MB is 32x smaller than a 50k x 768 fp16 embedding table (76.8 MB) and 1e2-1e5x smaller than a transformer
(1e8-1e11 parameters at 2 B each), but it is not "almost zero": it exceeds every register file, VTCM slice and SMEM;
it fits L2 (H100 50 MB) and a full 8 MB VTCM. The two sides decouple because angles are STREAMED: a sentence reads
sum_w P_w x 2 B (16-100 B) once, generates the states in registers, discards them; the lexicon lives wherever the
token-id gather reaches (L2 / VTCM / DDR, Zipfian hot words in cache). The working set (12.11) must be resident for
the whole contraction. Therefore the WORKING SET is the on-chip constraint (sets q_n, q_s) and the PARAMETER COUNT is
an off-chip capacity / bandwidth constraint (sets V and L) that never decides register residency. "Almost zero
memory" is true of the first and false of the second once V P_w exceeds ~1e5.

### 12.5 Low-rank relaxations: the verb as a matrix product operator

Replace the dense verb by a tensor train of bond dimension r:

    V[a, s, b] = sum_{r1 < r} sum_{r2 < r} A[a, r1] B[r1, s, r2] C[r2, b];  storage d r + r^2 d_s + r d  (vs d^2 d_s);
    rank(V viewed as (a) x (s, b)) <= r                                                                                  (12.13)

Contract the nouns first, then the core, never rebuilding V; two-stage form so B is read once in address order:

    u[r1] = sum_a n1[a] A[a r + r1]           d r MACs
    w[r2] = sum_b C[r2 d + b] n2[b]           r d MACs
    for (s) sigma[s] = 0;
    for (r2 = 0; r2 < r; ++r2) for (s = 0; s < d_s; ++s) { t = 0; for (r1 = 0; r1 < r; ++r1) t += u[r1] * B[r1 + r s + r d_s r2];
                                                           sigma[s] += t * w[r2]; }                          r^2 d_s + r d_s MACs   (12.14)
    total 2 d r + r^2 d_s + r d_s MACs; resident 2 r + d_s + 1 amplitudes with A, B, C streamed AND the nouns stored
    d-vectors (Sec. 1.6) streamed with A / C; + d for one materialised (angle-generated) noun at a time;
    else d r + r^2 d_s + r d resident.

Examples: d = 256, r = 8, d_s = 4: 4352 stored vs 262144 (60x); d = 1024, r = 16, d_s = 4: 33792 vs 4194304 (124x).
(12.9)-(12.10) are untouched: V is still bilinear and order-sensitive, only rank <= r is imposed.

Angle parameterisation (quantum, unitarity kept). r = 2^{q_r}; A[a, r1] = <a| U_A |r1> for r1 < r (first r columns of
a q_n-qubit unitary: an isometry), C[r2, b] = <r2| U_C |b> (first r rows), B = U_B |0> on 2 q_r + q_s qubits (C10). By
the bend-the-wire identity (Sec. 1.4, C11) A and C are never materialised:

    u[r1] = (U_A^T n1)[r1], r1 < r  (reversed, transposed gate list applied in place to n1; keep the first r entries; ||u|| <= 1)
    w[r2] = (U_C n2)[r2],  r2 < r  (||w|| <= 1);   sigma[s] = sum u[r1] B[r1 + r s + r d_s r2] w[r2],  |sigma[s]| <= 1
    P_V = 2 q_n L + 2 q_n L + 2 (2 q_r + q_s) L angles (Sim15);   resident 2^{q_n} + r^2 d_s + 2 r + d_s (sigma may overwrite u)   (12.15)

At q_n = 8, q_r = 3, q_s = 2, L = 2: 96 angles, 256 + 256 + 16 + 4 = 532 resident amplitudes for a verb whose dense form
is 2^18. Range bounds of Sec. 4.1 hold verbatim (unitaries and truncations never grow norms); shift and adjoint rules
apply; hardware-realisable (the truncation postselects the high q_n - q_r qubits on |0>). PRECISION: the truncation
is a postselection with generic survival r/d per noun; Monte Carlo (derivF/zmc.c) gives Z ~ 1/d^2 = 2^{-2 q_n} (the
same as the dense model at equal q_n), so (4.7) amplifies errors by 2^{q_n+1}: fp32 storage and accumulation for
q_n >= 5; Q1.15 only for q_n <= 4. At q_n = 8 the 532 amplitudes fit a VTCM slice in bytes (2128 B Q1.15) but the
model is fp32-only (4256 B): it belongs to the fp32 VTCM / SMEM tier.

Free complex entries (quantum-INSPIRED, unitarity dropped). A, B, C arbitrary complex arrays trained by gradient
descent; storage = the entries (4-8 B each), no generation flops. Lost: norm control, so Sec. 4.1 fails unless a
constraint is imposed. The right constraint is the OPERATOR norm (the isometry model has it for free):

    ||A||_op <= 1, ||C||_op <= 1, ||B||_F <= 1  ->  ||u|| <= 1, ||w|| <= 1, |sigma[s]| <= 1, every partial sum <= 1     (12.16)
    projection after each optimiser step: A /= max(1, sigma_max(A)) with sigma_max from 5-10 power iterations on the
    r x r Gram A^H A (d r^2 MACs), or clip singular values to 1 with the Jacobi SVD of Sec. 14.3 (d x r)

A Frobenius constraint ||A||_F <= 1 would also be range-safe but forces entries ~1/sqrt(d r) and Z ~ 1/(d^2 r^2)
(2.4e-7 at d = 256, r = 8: |sigma| ~ 3e-4, 10-30 LSB of Q1.15, relative error O(1) with the Z_min guard silent). Under
(12.16) Z ~ 1/d^2 as in the dense model and the fp32-only rule above applies; the free-entry model is fp32 only
(Q1.15 not applicable; the per-core scale-exponent variant is the fixed-point fallback). Also lost: parameter-shift
(no gates), the E[Z] heuristic (Z can be tiny by construction: the Z_min guard becomes load-bearing), and any
hardware-realisability claim; adjoint differentiation reduces to reverse-mode through (12.14).

Arbitrary local dimension d (qudits). Fields become mixed-radix digits: i = sum_j v_j stride[j], stride[j] =
prod_{j' < j} d[j']; v_j = (i / stride[j]) % d[j] (Hexagon has no hardware integer divide, ~20 cycles per library
call, UNVERIFIED) or an odometer that never divides:

    for (j = 0; j < n; ++j) v[j] = 0;  i = 0;
    loop { body(i, v);  for (j = 0; ; ++j) { i += stride[j]; if (++v[j] < d[j]) break; v[j] = 0; i -= d[j] * stride[j];
                                             if (j == n-1) goto done; } }
    done: ;                                                                                                             (12.16')

(traced for radices (3,2,3): visits 0..17 in address order, Sec. 18 T14.) What changes: scatter(x, fields) becomes a
mixed-radix decompose; the cup pairs digit v_A = v_B over k < d (<cup_d| = sum_k <k|<k|); bit masks and gate
butterflies are gone. What stays: parser, functor, fused loop structure and slot rule, readout, fp32 accumulation and
Z guard, (12.16). Angle generation of a general d-dimensional unit vector needs d - 1 real angles (2 d - 2 with
phases: the hyperspherical chain v_0 = cos t_0, v_j = cos t_j prod_{j'<j} sin t_{j'}, v_{d-1} = prod sin t_{j'}),
exactly (12.1)-(12.2): for qudits the angle table is as large as the vector; there is no compression to be had.

### 12.6 Multi-layer (deep) composition

Second layer: sentence states are noun-like inputs to a discourse word D of type s^r s s^l (3 q_s qubits):

    sigma_2[t] = sum_{s1, s2} sigma_hat_1[s1] D[s1 + d_s t + d_s^2 s2] sigma_hat_1'[s2]                                  (12.17)
    memory(2 layers, n sentence inputs) = max( 2^Q + 2^{q_n} + (n-1) d_s,  2^{3 q_s} + n d_s )   -- a maximum, not a sum,
    because layer 1's working set is dead when layer 2 starts; only the finished d_s-vectors cross; (4,2), n = 2: 1044   (12.18)

Measure-and-reprepare: carry only p_1[k] (d_s reals) and re-prepare a fresh state. Simulator path: phi'[k] =
sqrt(p_1[k]) (d_s square roots on the scalar core, no gates, no angles), fed to layer 2 as a stored-vector word.
Hardware path (off-device): Givens angles t_j = atan2( sqrt(sum_{k > j} p_k), sqrt(p_j) ) (no division, no acos of an
argument > 1), d_s - 1 angles at 2 B:

    layer boundary: p_1 = readout(sigma_1);  phi' = sqrt(p_1) (simulator) or generate(encode(p_1)) (hardware)           (12.19)

What it buys: the inter-layer state is classical (phases lost: a nonlinearity, like an activation), the layer-2 kernel
is the unmodified single-layer kernel, and memory is the single-layer maximum at every depth. DisCoCirc caveat: text
with NOUN wires persisting across sentences has working set 2^{q_n x (live nouns)}: depth is free, WIDTH
(simultaneously open wires) is exponential, and that is where "recursion beyond simple adjoints" meets the memory law
(Sec. 14).

### 12.7 Capacity verdict, with numbers

    memory (amplitudes) ~ 2^{k q}  (k = arity of the widest word, verb k = 3);  capacity (free-tensor dof) ~ 2^{k q + 1} - 2,
    realised dof <= P_w (12.5);  factorised (12.13): memory ~ k d r + r^2 d_s, capacity rank-r bilinear maps on C^d        (12.20)

(1,1): a 2-class classifier on d = 2 nouns; a transitive path has 8 (IQP L = 1) to 12 (Sim15 L = 1) angles; working set
10 complex (80 B fp32 / 40 B Q1.15). It learns word order (12.10) and little else; a toy on 100-sentence data sets.
(4,2): verb 1024 amplitudes = 8 KB fp32 / 4 KB Q1.15, d = 16, 4 classes, Sim15 L = 2 -> 40 angles per verb (3.9 % of
the real manifold). As a free tensor it is a classical DisCoCat model at d = 16; published classical tensor models
used d ~ 300-2000 (1e5-1e9 real entries per verb): "comparable to a small classical tensor model" means small.
Open-domain: d = 256-1024 (q_n = 8-10) with contextual word states puts a dense verb at 2^17-2^21 amplitudes (q_s = 1):
0.5 MB fp16 to 16 MB fp32 per verb -- 500-16000x over registers or an 8 KB VTCM slice, 2-70x over the largest SMEM;
one fp16 verb at q_n = 8 fits a full 8 MB VTCM or L2, a 1000-verb lexicon (0.5-16 GB) does not. Factorised (12.13) at
r = 8-16: 4-34 K amplitudes per verb (35-270 KB fp32; d + 2 r + d_s resident when streamed) -- but that IS a classical
low-rank tensor model, non-contextual and grammar-bound (12.2), and no q_n, q_s, L or r changes the class (12.9).
"Almost zero memory" (register / VTCM residency, Q <= 9-10) and "transformer-competitive on open-domain benchmarks"
(contextual, non-multilinear, 1e8+ parameters) are incompatible for this class -- not by a constant factor but by
(12.20) against the multilinearity of 12.2 (made precise in Sec. 15.2).

Reachable per tier (C14): REGISTER (CUDA ~1 KB/thread; HVX 4 KB): (q_n, q_s) <= (2,2) fp32 or (3,1) real on CUDA;
(3,2) safe / (4,1) marginal on HVX amplitude-per-lane; (1,1) sentence-per-lane: 2-4-class classification over the
Sec. 11 grammar with lexicons of any size (streamed); the honest use is small-domain intent / sentiment / relation
classification on grammatical short sentences. VTCM slice 8 KB: (4,2) Q1.15 (q_n = 4 is also the precision limit of
Q1.15); the (12.15) verb at q_n = 8 fits in bytes only. SMEM 48-227 KB: dense (5,2)-(6,2) fp32, or factorised
d = 256-1024, r = 8-16 with cores streamed: classical-tensor-DisCoCat scale, real-vocabulary nouns, non-contextual.
HBM: dense d = 256-1024 verbs (1-16 MB each); full classical tensor DisCoCat capacity with quantum-style
normalisation; on open-domain benchmarks bounded by 12.2 (i)-(iv), not by memory.

---------------------------------------------------------------------------------------------------------------

## 13. Hybrid architecture: open vocabulary and context without giving up the compositional core

This is Sec. 13 (addendum part G); equations are (13.n). Both criticisms are accepted. This section replaces the
lexicon by a GENERATOR of angles, adds context by three mechanisms, and states the ceiling. It reuses the fused
tensordot (Sec. 1.3), the adjoint rule (5.6)-(5.8) and the tiers of C14. Symbols: m = feature dimension, P_c = angle
count of lexical class c (C10), V = vocabulary, B = hash buckets, n_w = words per sentence, ell = word length in bytes.

### 13.1 Angles are generated, not stored

The interface between any classical front end and the compositional core is the angle vector, never an amplitude:

    theta_w = f_c(x_w),  c = class(w) in {n, n n^l, n^r s, n^r s n^l, s^r s, ...},  x_w in R^m
    linear (default):  z = W_c x_w + b_c,  theta = z  (any real is a valid angle; the loss is exactly 4 pi-periodic, Sec. 5.6)   (13.1)
    bounded (optional): theta = 2 pi tanh(z)  -- covers one 4 pi period but dL/dz = dL/dtheta 2 pi (1 - tanh^2 z) vanishes
                        for |z| > 3 (dead zone); use only if a bounded output is wanted numerically                             (13.1')
    turn encoding, fp32 path:    k = (uint16_t)( (int64_t) llround( fmod(z, 4 pi) * (65536.0 / (4 pi)) ) & 0xFFFF )
        (fmod, then a SIGNED 64-bit round, then mask; never cast a float to uint16 directly: negative or out-of-range
         float -> unsigned conversion is undefined behaviour in C/C++; guard: if (!isfinite(z)) z = 0.
         Check values: z = -1.0 -> llround -5215 -> k = 60321;  z = 0.8 -> 4172 (Sec. 8);  z = 13.0 -> fmod 0.433629 -> 2261)
    integer path (v6x, no vector fp): acc = int32 dot(W_c row, x_w) (exact for m <= 2^16);  k = (uint16_t)( ((int64_t) acc * Cc) >> 32 )
        with Cc = (int64_t) llround( s_E s_W 65536.0 / (4 pi) x 2^32 ) stored once per class; arithmetic shift, low 16 bits = the turn   (13.2)

Why angles and not amplitudes (three reasons that hold; the "QR per word" argument does not: the fused tensordot
consumes only phi_w, and an amplitude interface would need one O(2^Q) normalisation plus the clamp ||phi|| <= 1 for
the Q1.15 range proof, its quantisation error being the Sec. 4.7 word term with 2^-8 in place of 2^-16): (a) P_c <<
2^{Q+1} - 2, so f_c is small and the per-word cost is P_c m MACs, not 2^Q m; (b) the circuit form (Sec. 1.4), the
DisCoCirc gate (C17) and the "legal circuit" property of 13.6 need a UNITARY, which angles give for free and
amplitudes give only via a Householder completion (O(2^{2Q}) ops) for states or a QR (O(2^{3Q}) ops, 2^{2Q} memory)
for gates; (c) exact 4 pi-periodicity makes any unbounded linear output a valid angle, so no nonlinearity, clamp or
normalisation is needed anywhere between the front end and the core. The generated (cos, sin) pair is unit-norm to
<= 4e-7 (fp32 polynomial, Sec. 4.4) or <= 4.3e-5 (Q2.14 entries, Sec. 4.3); the C5 angle quantisation adds <= 4.8e-5
state error per gate. No int8 error in W_c or E can produce a non-unitary gate.

Type classifier. The parser needs a type string for an unseen word: g([x_w, c_w]) = argmax(W_t [x_w, c_w]), W_t
in R^{T x 2m} (T = number of type classes; 5 x 64 int8 = 320 B at m = 32 with the context vector of 13.2(b), 160 B
without). It is a POS tagger; its error, not the head, bounds parse accuracy, and an isolated word's character
n-grams cannot separate "run" (n) from "run" (n^r s), so context is required. On parse FAIL retry with the
second-best tag of the word with the smallest margin, at most n_w retries; if all fail, output uniform
probabilities and D = 0 (Sec. 1.2 convention). Lexicon storage is replaced by

    bytes(f) = sum_c P_c (m + 1) x bytes/param;   (1,1), IQP L = 2: sum_c P_c = 3 + 2 + 2 + 4 + 2 = 13;
    m = 32: int8 416 B + 52 B fp32 bias; fp32 1,716 B.   Compare lexicon: 2 B x V x mean P_w = 100 KB at V = 1e4          (13.3)

Feature sources (bytes):

(a) Hashed character n-grams (fastText-style). Word with boundary markers "<w>", G_w = all byte n-grams with
n in [3, 6]; |G_w| = sum_{n=3}^{6} max(0, ell + 3 - n) = 4 ell - 6 for ell >= 3 (ell = 1: 1; ell = 2: 3; ell = 7: 22).

    x_w = (1/|G_w|) sum_{g in G_w} E[ h(g) & (B-1) ],  E: B x m int8 with one global fp32 scale;  |G_w| >= 1 always       (13.4)
    h = FNV-1a 32-bit: h = 0x811C9DC5; for each byte: h ^= byte; h *= 0x01000193   (check: h("<the>") = 0xFCF0F1C0, bucket 29120 at B = 2^15)
    integer mean: x_w[j] = sat_int8( (sum_j * inv + 2^15) >> 16 ),  inv = round(2^16 / |G_w|)
    bytes(E) = B m:  B = 2^15, m = 32: 1,048,576 B (1 MB);  B = 2^13, m = 16: 131,072 B (128 KB)

A 50 K-word vocabulary has 1e5-1e6 distinct 3-6-grams, so B = 2^15 means >= 10-way bucket sharing (fastText's default
is 2e6 buckets: B = 2^20 x m = 32 = 32 MB is the faithful point); the accuracy cost of B = 2^15 is UNVERIFIED.
Zero-table variant: E is never stored but generated by a fixed 32-bit mixer (identical on trainer and device):

    uint32_t mix32(uint32_t x) { x += 0x9E3779B9u; x ^= x >> 16; x *= 0x85EBCA6Bu; x ^= x >> 13; x *= 0xC2B2AE35u; x ^= x >> 16; return x; }
    E[h][j] = (int8_t)( mix32( (uint32_t) h * (uint32_t) m + (uint32_t) j ) >> 24 ),  all arithmetic uint32 mod 2^32
    test vectors: mix32(0) = 0x92CA2F0E (byte -110);  mix32(1) = 0x96A0F96B (-106);  mix32(0x12345678) = 0x047660EA (4)        (13.4')

Then (13.4) is a fixed random projection of the n-gram bag (Johnson-Lindenstrauss): two distinct n-grams have
normalised inner product ~1/sqrt(m) = 0.18 at m = 32, so 22 active n-grams interfere heavily in 32 dimensions; the bag
is faithfully represented only for m >> |G_w| (m = 256: 0.06). Cost: memory 0 B; ops |G_w| m mixer evaluations at ~9
scalar ops each (22 x 32 x 9 ~ 6 K scalar ops per token, or ~250 HVX vector ops since a 32 x 32-bit low multiply is a
vmpyie / vmpyio pair; UNVERIFIED, no Hexagon toolchain here) instead of a 704 B gather; nothing at the character
level is learnable, all capacity sits in W_c. Use it when memory is the binding constraint and m >= 128 is affordable.

(b) Quantised pretrained embedding table: V x m int8 + per-row fp16 scale + a word -> row map (open addressing,
(hash u32 + row u16) per slot at load 0.75): bytes = V m + 2 V + 6 V / 0.75. V = 50,000, m = 64: 3,200,000 + 100,000 +
400,000 = 3,700,000 B (3.5 MB). Not "almost zero"; OOV falls back to (a).

(c) Byte-pair pieces: piece table V_s x m int8 + merge list (V_s pairs of u16) + strings (~6 B each): V_s = 8,192,
m = 32: 262,144 + 32,768 + 49,152 = 344,064 B (336 KB); x_w = mean of the piece rows. Floor of the family: raw bytes,
(256 + 16 position rows) x 32 = 8,704 B, a bag of letters (UNVERIFIED usefulness beyond spelling-level tasks).

### 13.2 Context

(a) DisCoCirc meaning updating (functor C17, full treatment Sec. 14). Nouns are persistent wires; a sentence is a
process on the wires it mentions; for a text with N distinct nouns (pre-pass over the text; two mentions are the
same noun iff their tokenised byte strings are equal, no pronoun resolution -- UNVERIFIED that this suffices; Sec.
14.5 gives the coreference interface) the text state lives on N q_n qubits:

    |Psi_0> = (x)_{j} U_n(theta_{noun_j}) |0>^{q_n};   |Psi_{t+1}> = G_t |Psi_t>,  G_t = U_sent_t(theta) on the mentioned fields   (13.5), (13.6)
    postselected sentence (ancilla a): |Psi_{t+1}> = (<0|_a (x) I) U_t (|Psi_t> (x) |0>_a) / sqrt(p_t)
    CPTP form: rho_{t+1} = Tr_a[ U_t (rho_t (x) |0><0|_a) U_t^dag ]                                                          (13.7)

A transitive verb is a 2 q_n-qubit unitary from the same ansatz family (Sim14 on Q = 2 q_n). DisCoCat uses U_V(theta)|0>
(the first column), DisCoCirc uses U_V(theta) itself: same angles and ansatz, DIFFERENT objects; weights are not
interchangeable (the exact Choi correspondence relates the 2^{2 q_n} DisCoCat state to a q_n -> q_n map M[b][a] =
V[a, b], non-unitary in general, not to the 2 q_n-qubit gate). Memory: unitary sentences 2^{N q_n} complex (N = 8,
q_n = 1: 256 = 2 KB fp32 / 1 KB fp16; N = 12: 32 KB / 16 KB; N = 8, q_n = 2 or N = 16, q_n = 1: 512 KB / 256 KB);
postselected ancilla form 2^{N q_n + 1} (one ancilla, reset between sentences); CPTP form (13.7) 2^{2 N q_n} (N = 8,
q_n = 1: 65,536 complex = 512 KB) -- do not use it; use the pure form with a SINGLE final normalisation. With
per-sentence postselection p_total = prod_t p_t and (4.7) amplifies the forward error by 1/sqrt(p_total): at
p_t ~ 1/2 a 20-sentence text has 2^10 amplification, breaking fp16 / Q1.15 after 2-4 sentences (Sec. 4.5 rule 3);
texts longer than log2(1/p_min) / log2(1/p_t) sentences must use unitary sentences (13.6) or fp32 with
renormalisation after every sentence (Sec. 4.5 rule 2), or the trajectory unravelling (sample the ancilla outcome
per sentence, memory 2^{N q_n}, unbiased for N_c and D over T trajectories). Ops per sentence: fused 2 q_n-qubit
block on 2^{N q_n} amplitudes = 4 x 2^{2 q_n} x 2^{N q_n} real FMA (4,096 at N = 8, q_n = 1) with the dense verb
2^{4 q_n} complex (128 B / 2 KB at q_n = 1 / 2) generated transiently per sentence, or gate-by-gate (Sim14 L = 2,
Q = 2: 16 gates x 2^7 pairs x 8 = 16 K FMA). Ceiling per target (S(W), Sec. 7): N q_n <= 9 fp32 / 10 fp16 under the
8 KB VTCM slice; 12 / 13 Adreno local; 13-14 / 15 SMEM (14 fp32 only on 163-227 KB parts); 19 / 20 on an 8 MB VTCM.
Beyond it: the mean-field variant keeps one density matrix per noun, rho_j (2^{2 q_n} complex, 32 B / 128 B), applies
the verb to rho_i (x) rho_j (working set 2^{4 q_n} complex = 128 B / 2 KB, gate-by-gate on both sides, 2x the gate
cost) and traces the partner out: memory linear in N, no inter-noun entanglement between sentences; or the MPS of
Sec. 14.3. This is the only mechanism in this section that carries cross-sentence dependencies through the QUANTUM
state (the recurrent h_t of (13.9) carries them classically when not reset).

(b) Classical context gate. theta_w = f_c([x_w, c_w]) with W_c in R^{P_c x 2m} ((13.3) becomes sum_c P_c (2m + 1) =
845 B int8 at m = 32):

    window:     c_w = (1/2r) sum_{0 < |d| <= r} x_{w+d},  x = 0 outside the sentence, denominator stays 2r  (0 extra params)   (13.8)
    recurrent:  h_t = tanh(A h_{t-1} + B_m x_t),  c_w = h_t  (bidirectional: two states, 4 m^2 params)                      (13.9)

Memory O(m^2): A, B_m int8 at m = 64: 8,192 B (bidirectional 16,384 B); fp32 for training 32 KB. Ops per token
2 m^2 = 8,192 int8 MACs + 2 m P_c for f. One string ("bank") now yields different angles in different sentences;
nothing on the quantum side changes and the head's working set is unchanged.

(c) Density-matrix word meanings. rho_w = sum_i p_i |w_i><w_i| (ambiguity as a mixture), 2^{Q_w} x 2^{Q_w} Hermitian.
Composition is the same tensordot on doubled indices (C7 index map, primes = bra side; R row index s + 2^{q_s} b,
column likewise):

    Sigma[s][s'] = sum_{a,a',b,b'} rho_1[a][a'] rho_V[(a,s,b)][(a',s',b')] rho_2[b][b'],  p[k] = Sigma[k][k] / Tr Sigma          (13.10)
    step 1: R[(s,b)][(s',b')] = sum_{a,a'} rho_1[a][a'] rho_V[(a,s,b)][(a',s',b')]     2^{2(q_s+q_n)} outputs x 2^{2 q_n} terms
    step 2: Sigma[s][s'] = sum_{b,b'} R[(s,b)][(s',b')] rho_2[b][b']                  2^{2 q_s} outputs x 2^{2 q_n} terms

Memory squares. Verb 2^{2(2 q_n + q_s)} complex: (1,1) 64 = 512 B fp32 (pure 64 B); (2,1) 1,024 = 8 KB (pure 256 B).
Working set rho_1 + rho_V + R + rho_2: (1,1) 4 + 64 + 16 + 4 = 88 complex = 704 B fp32 / 352 B Q1.15 (5.5x the 16-
amplitude working set of Sec. 7, 8.8x the in-place 10); (2,1) 16 + 1,024 + 64 + 16 = 1,120 complex = 8,960 B / 4,480 B
(23x the 48 of Sec. 7, 31x the in-place 36) -- out of the HVX register file, out of the 8 KB VTCM slice in fp32 (in
under Q1.15), SMEM-resident. Layouts: (1,1) density on HVX amplitude-per-lane (5.5 vectors fp32 / 2.75 Q1.15), on CUDA
warp-per-sentence (Layout B); never the sentence-per-lane layout (176 vectors). Ops: (1,1) 64 + 16 = 80 complex MACs vs
12 pure (6.7x); (2,1) 1,024 + 64 = 1,088 vs 40 (27x). Cheaper alternative: (13.10) is multilinear, so

    N_c = sum_i (prod_j p_{i_j}) |sigma_c^{(i)}|^2,   D = sum_i (prod_j p_{i_j}) Z^{(i)},   p[c] = N_c / D              (13.10')

exactly -- weighted averages of the UNNORMALISED pure quantities, a ratio of averages, not an average of
probabilities. Exact cost K^{n_w} pure runs (K = 2, N TV N: 8 runs ~ the density kernel's 6.7x at (1,1); K = 3 at (2,1):
27 = break-even), memory unchanged plus 2^{q_s} + 1 fp32 accumulators; Monte Carlo (i_j ~ p_j, T runs) is unbiased for
N_c and D, and p[c] is a ratio estimator with O(1/T) bias; the correlated mixture (one shared index for all words,
K runs) is a smaller valid model. Any rho of rank r is a mixture of r pure states, so K = 2^{Q_w} loses nothing
PROVIDED the word ansatz reaches rho's eigenvectors (not ansatz 0 at L = 1, whose amplitudes all have modulus
2^{-Q/2}, C10; Sim14 / Sim15 at sufficient L: UNVERIFIED). With the generator of 13.1: K heads W_c^{(i)} and one
logit head W_pi in R^{K x m}, p = softmax(W_pi x_w): K P_c (m+1) + K m parameters per class.

### 13.3 Data re-uploading: m features into a Q-qubit word without more amplitudes

Layer l of the word ansatz (any C10 ansatz; for a Q = 1 word one layer = the Rx Rz Rx block) is applied with angles

    theta^{(l)} = theta_l + W_l x_w,  l = 1..L,  |w(x)> = U^{(L)}(theta^{(L)}) ... U^{(1)}(theta^{(1)}) |0>^Q               (13.11)
    parameters per class: L P_L (m + 1), P_L = angles per layer (= P_w / L of C10);  P_L = 3, m = 64, L = 3: 585 (2,340 B fp32, 585 B int8)

Since exp(-i (theta + w.x) G/2) = exp(-i theta G/2) exp(-i (w.x) G/2) and every C13 generator has eigenvalues in
{0, +-1}, each amplitude is a sum of terms e^{i omega . x} with omega in {(1/2) sum_g s_g w_g : s_g in {-1, 0, +1}}
over the L P_L parameterised gates; probabilities are trigonometric polynomials with frequency set Omega - Omega;
with tied directions w_g = w this is a degree-L P_L polynomial in w.x (Perez-Salinas et al. 2020; Schuld, Sweke,
Meyer 2021 for the Fourier statement -- UNVERIFIED citations, the derivation from C13 stands on its own).
Expressivity in x grows with L while the working set stays 2^Q amplitudes (layers are applied in place); extra
P_L m parameters per layer are read once per token; ops L P_L m int8 MACs per token plus L times the gate count.

Honest ceiling of this whole direction. A pure Q-qubit state has 2^{Q+1} - 2 real dof; a noun at q_n = 1 is a point on
the Bloch sphere, 2 reals, however rich x_w is: re-uploading changes WHICH 2 reals, not how many. Real language
tasks plausibly need q_n = 4..6 (30..126 reals per noun, the scale of 32-128-dimensional static embeddings;
UNVERIFIED heuristic). Working set of the fused tensordot 2^{2 q_n + q_s} + 2^{q_n} + 2^{q_n + q_s} + 2^{q_n}: (5,2)
4,288 complex = 33.5 KB fp32 / 16.8 KB fp16 -- fits SMEM (48 KB static), Adreno local in fp16 only, a VTCM
reservation >= 40 KB, not the 8 KB slice; (6,2) 16,768 complex = 131 KB / 65.5 KB -- dynamic SMEM (163 / 227 KB parts)
or VTCM >= 160 KB only. Verb preparation dominates: Sim14, L = 2, Q = 12: 96 gates x ~12 K real FMA ~ 1.2 M FMA per
sentence vs 4,224 complex MACs for the contraction. Beyond q_n = 6 the model is out of on-chip memory on every
target and the "almost zero" claim is false for the head as well.

### 13.4 End-to-end training without Python

Per sentence the adjoint pass (5.6)-(5.8) yields grad_local[k] per gate; scatter-add with sign_k (Sec. 5.3, 5.5)
gives dL/dtheta_w[i], accumulated per UNIQUE word of the batch by a segmented reduction (keys = (word_hash << 8) |
param index, radix-sorted per batch, O(batch x n_w x P_c), then one segmented sum; bitwise reproducible, unlike
atomicAdd). Then per unique word once:

    (13.1):  dL/dz = dL/dtheta;   (13.1'): dL/dz_i = dL/dtheta_i 2 pi (1 - tanh^2 z_i)                                     (13.12)
    dL/dW_c += (dL/dz) [x_w, c_w]^T  (P_c x 2m outer product);  dL/db_c += dL/dz;  dL/dx_w = W_c[:, 0:m]^T dL/dz
    learned E: for g in G_w: dL/dE[h(g)] += dL/dx_w / |G_w|   (E_train is fp32, 4 B m B; rows collide (#distinct n-grams / B)-way;
               int8 + one global scale at export)
    re-uploading: dL/dW_l += (dL/dtheta^{(l)}) x_w^T per layer (the adjoint gives per-gate, hence per-layer, gradients)
    recurrent gate (13.9), backward through time (store h_1..h_T, T m fp32 per direction):
        delta_t = W_c[:, m:2m]^T dL/dz_t + A^T ( delta_{t+1} . (1 - h_{t+1}^2) );  g = delta_t . (1 - h_t^2);
        dL/dA += g h_{t-1}^T;  dL/dB_m += g x_t^T;  window (13.8): dL/dx_{w+d} += dL/dc_w / (2r)

Adam (5.9) runs unchanged over [W_c, b_c, E, A, B_m, W_l]; state 16 B/param fp32. Sizes: f at (13.3), m = 32: 13 x 33 x 16
= 6,864 B; learned E at B = 2^15, m = 32: 16 MB during training (1 MB int8 at export); recurrent m = 64: 128 KB; hidden
states 256 B/token per direction at m = 64. The quantum side has no per-word parameters left; only the adjoint state
vectors (24 x 2^W B, Sec. 5.3). Fixed-point safety is automatic: the core consumes uint16 turns or fp32 theta and both
map to exact-norm rotations. Quantisation of E and W_c to int8 moves angles by |d theta| <= ||dW_c|| ||x_w|| +
||W_c|| ||dx_w|| (times 2 pi on (13.1')); on the linear path |z| is unbounded, so keep |z| = O(1) by an L2 penalty on
W_c (or use (13.1')) or int8 export of W_c is unsafe; verify |d P_c| after export against the Sec. 4.7 budget on the
validation set. Wrap theta mod 4 pi only at export (Sec. 5.6).

### 13.5 NPU inference pipeline (hybrid), per token

    1. tokenise: byte scan, ~ell scalar ops; boundary markers.
    2. n-gram hashes: ~4 ell FNV updates (2 scalar ops each): ~60 ops at ell = 7.
    3. gather |G_w| rows of E: 22 x 32 B = 704 B (VTCM if the table is resident on a >= 1 MB reservation, v73+; under the
       8 KB slice E is off-chip: 22 x 64 B = 1,408 B of L2 / DDR traffic per token -> prefer the zero-table variant there).
    4. sum rows: int8 -> int16 widening, 6 vector adds, 2 rotate-and-add folds, scale, pack: ~15-20 HVX ops (estimate).
    5. f_c: W_c x_w: P_c x m int8 MACs = 96 (noun) .. 384 (Sim14 TV, L = 1) MACs; one vrmpy(Vu.b, Vv.b) does 128 int8 MACs
       into 32 int32 lanes (256 in the vector-pair form; hvx_protos.h), plus a 5-op lane-reduction tree: ~6-12 vector ops
       per token; or one HMX int8 tile per 32 tokens when batched (X: 32 x m times W_c^T) -- the dense GEMM shape HMX is
       for, unlike the butterflies of Sec. 3.3 (tile rates UNVERIFIED).
    6. angles: (13.2) is a multiply and a shift per angle; (13.1') adds ~5 ops (rational tanh).
    7. cos/sin: Sec. 4.4 polynomial ~20 FMA per angle (60-240 per token) or the 514 B table.
    8. head: word preparation + fused tensordot: (1,1) IQP L = 2: verb 6 H x 32 + 4 CRz x 16 = 256, two nouns 2 x 24 = 48,
       contraction 12 complex MACs = 48: ~350 real FMA per sentence (~120 per token); (5,2): ~1.2 M.

Per token at (1,1): ~1.5 K front-end ops and ~1 KB moved against ~120 head ops: the front end dominates ~13:1 and is
bandwidth-bound on the gather (zero-table: compute-bound, ~6 K ops, 0 B). At (5,2) the head dominates 100:1 and is
FMA-bound in verb preparation. Resident objects above 1 KB: E (128 KB - 1 MB), A / B_m (8-16 KB), W_c at (5,2)
(10 KB at m = 32 to 117 KB at m = 384). Off-chip traffic per sentence: the token bytes in, 2^{q_s} + 1 outputs out,
plus the E gathers when E is not resident.

### 13.6 A frozen transformer as the front end, the compositional core as the head

When the front end is an int8 transformer encoder, x_w is the contextual token vector (d = 384-768), 13.2(b) is free
and (13.1) has W_c in R^{P_c x d}. Memory is then the encoder's: 6-layer d = 384 (MiniLM-L6 class) ~22 M params =
22 MB int8 plus activations; BERT-base ~110 MB int8 (both UNVERIFIED sizes). The head stays at (13.3) with m = d:
(1,1) 13 x 385 = 5,005 params = 5 KB int8; (5,2) Sim14 L = 2: sum_c P_c = 40 + 80 + 56 + 96 + 32 = 304 (noun Q = 5,
ADJ Q = 10, IV Q = 7, TV Q = 12, ADV Q = 4), x 385 = 117,040 params = 117 KB int8 (58 KB at L = 1), working set 33.5 KB.
The "almost zero memory" claim then applies to the head only; this is the configuration in which the system can be
competitive on real tasks, and the honest statement is that the capacity comes from the encoder. What the head adds
by construction, not by training: (i) exact compositional semantics: sigma = C_parse(phi_1, .., phi_{n_w}), the fixed
multilinear map (1.3) dictated by the parse, no learned mixing; (ii) word-order and role sensitivity: "Alice loves
Bob" and its reversal differ whenever n_A^T (V_s - V_s^T) n_B != 0 for some s, i.e. generically, whereas a pooled head
is order-blind; (iii) an interpretable Hilbert-space meaning with a metric: 2^{q_s} complex amplitudes, fidelity
|<sigma_1|sigma_2>|^2 / (Z_1 Z_2) (Sec. 1.5), a Born-rule readout with the postselection weight D reported per
sentence; (iv) the head is a legal circuit (adjoint- and shift-differentiable, Sec. 5), so the same weights run on
quantum hardware if that is ever worth doing. What it does not give: a wide sentence embedding. At q_s = 1-2 the
sentence is 2-4 complex numbers -- adequate for K <= 2^{q_s}-class decisions, inadequate for retrieval against
384-768-dimensional encoders; q_s = 9-10 would match, but the verb 2^{2 q_n + q_s} at q_n = 5 is 2^19-2^20 complex:
q_s = 9 is 4 MB fp32 / 2 MB fp16 (a VTCM reservation of that size, or GPU global memory), q_s = 10 is GPU global
memory only (8 MB fp32 is the entire VTCM), and its preparation costs ~0.5-2 G FMA per sentence. For wide embeddings
use the DisCoCirc noun register (13.6) as the representation (2^{N q_n} amplitudes, 2 KB at N = 8, q_n = 1) instead
of widening s.

Summary of ceilings (bytes at which each becomes possible): open vocabulary, zero table: 0 B (+ ~0.5 KB of f);
learned character features: 128 KB - 1 MB; pretrained vectors: 3.5 MB; cross-sentence context with entanglement:
2^{N q_n} x 8 B (2 KB - 512 KB); ambiguity as density matrices: 5.5x / 23x the pure working set (704 B / 8.75 KB) or
K^{n_w} pure runs at unchanged memory; noun capacity comparable to small embeddings: q_n = 5-6, head working set
33.5-131 KB; competitive on real benchmarks: encoder 20-110 MB in front of a head of 5-117 KB. Below q_n = 4 with
any front end, and above q_n = 6 on any on-chip budget, this class of model cannot do real-language semantics, and
the reason is the count 2^{q_n+1} - 2 of reals per noun, not the lexicon.

### 13.7 Export of the generator (storage side of the interface; extends Sec. 6.3 and C15)

The device loader of Sec. 6.2 consumes a hybrid model only if W_c, b_c, the codec constants of (13.2), the type classifier,
the feature table (or its absence) and the context gate have a byte layout. Header flags.bit3 = 1 (C15) marks the block;
version += 1 as in C15. All multi-byte fields little-endian; every offset below is RELATIVE TO THE GENERATOR HEADER, which
starts at the first 8-B-aligned offset after the Sec. 6.3 angle table (32 + 20 V + P_total x (2 or 4), rounded up).

    Generator header (16 B):
      @0  magic 'Q','N','L','G';  @4 m u16 (feature dim);  @6 B_log2 u8 (0 = zero-table mix32 (13.4'); else E has 2^B_log2 rows);
      @7  nclass u8 (lexical classes, = T type classes: the classifier's argmax row t selects class record t);
      @8  ctx u8 (0 none; 1 window (13.8), r = @10; 2 recurrent (13.9), m_h = @10; bit7 set = bidirectional);
      @9  feat u8 (0 = hashed n-grams (13.4), 1 = embedding table (13.1 b, external file), 2 = byte pieces (13.1 c), 3 = encoder (13.6));
      @10 r_or_mh u16;  @12 s_E fp32 (global int8 scale of E; unused when B_log2 = 0: the mixer bytes are already the int8 values).
    Class record c (32 B x nclass, starting @16):
      @0  P_c u16 (angles of the class = C10 P_w of its type string and ansatz);  @2 m_in u16 (m, or 2 m with a context gate);
      @4  off_W u32 (int8 W_c, row-major P_c x m_in, row i = angle i of the C10 layout);  @8 off_b u32 (fp32 b_c[P_c], 4-B aligned);
      @12 s_W fp32 (per-class int8 scale of W_c);  @16 Cc int64 (the (13.2) integer-path constant, 8-B aligned by construction);
      @24 type_sig u8[6] (Sec. 6.3 encoding with the C15 base codes; this IS the class's type string, C7 order);
      @30 ansatz_id u8 (C10; 0xFE never occurs here: spiders have no angles);  @31 L u8 (layers; re-uploading (13.11): L blocks
          of W_l follow each other inside W_c, rows l P_L .. (l+1) P_L - 1).
    Type classifier (starts @16 + 32 nclass):  int8 W_t[nclass][m_in], then fp32 s_t;  t = argmax_t sum_j W_t[t][j] x[j].
    E (B_log2 > 0):  @off_E = next 4-B-aligned offset: int8 E[2^B_log2][m], row = FNV-1a(g) & (B - 1) (13.4).
    Context (ctx = 2):  int8 A[m_h][m_h], int8 B_m[m_h][m], fp32 s_A, fp32 s_B;  bidirectional: a second copy follows.
    Blobs: W_c (int8) and b_c (fp32, 4-B aligned) for c = 0 .. nclass-1 in class order, at the recorded offsets.
    Angle table of Sec. 6.3 then holds ONLY function-word / spider / OOV-tag blocks; content words have param_offset =
    0xFFFFFFFF ("generate") or no word record at all.

    Sizes (B):  16 + 32 nclass + nclass m_in + 4 + sum_c P_c (m_in + 4) [+ 2^B_log2 m] [+ m_h^2 + m_h m + 8 (x2 bidirectional)]
      (1,1), IQP L = 2, m = 32, zero-table, no context, nclass = 5, sum_c P_c = 13:    16 + 160 + 164 + 13 x 36 = 808 B
      + learned E at B = 2^15:                                                        + 1,048,576 B (1 MB)
      + recurrent gate m_h = m = 32 (m_in = 64):                                      16 + 160 + 324 + 13 x 68 + 2,056 = 3,440 B
      (5,2), Sim14 L = 2, m = d = 384 (encoder front end), nclass = 5, sum_c P_c = 304:  16 + 160 + 1,924 + 304 x 388 = 120,052 B = 117.2 KB
    (all int8 row lengths above are multiples of 4, so no alignment padding occurs in these four cases; in general pad
    each fp32 field to 4 B and count the padding).

Loader checks (fail the load, never silently continue): magic; nclass == number of distinct type_sig records; every off_W /
off_b inside the block; Cc == (int64_t) llround( s_E s_W 65536.0 / (4 pi) x 2^32 ) recomputed from the stored scales
(EXACT integer equality: a mismatched codec constant moves every angle); T19's three codec vectors run on class 0
(z = -1.0 -> k = 60321, 0.8 -> 4172, 13.0 -> 2261) with x_w chosen so that acc = 1000 corresponds to z = 0.8; and, when
B_log2 = 0, mix32(0) = 0x92CA2F0E (13.4'). Trainer and device share this one file; the trainer's fp32 master copy of
W_c, b_c, E, A, B_m is not exported.

---------------------------------------------------------------------------------------------------------------

## 14. Discourse and bounded-memory contraction: DisCoCirc text circuits and the MPS

Conventions C1-C17 apply. Noun wire i (i = 0..N-1) is the q_n-qubit field [q_n i, q_n (i+1)) of a text register of
W = q_n N qubits (C17: first-mentioned noun in the lowest bits); d = 2^{q_n}. Verification programs: derivH/jsvd.c
(one-sided Jacobi SVD), derivH/mpstest.c (MPS two-site update vs exact statevector in C1 bit order), verH/mps_nan.c,
jsort.c (the two failure modes fixed below), addendum/t18b.c (the 4x4 SVD test vector of Sec. 18).

### 14.1 DisCoCirc: nouns as persistent wires, sentences as gates

    Psi_0[i] = prod_{j=0}^{N-1} phi_{n_j}[ (i >> (q_n j)) & (d-1) ],   phi_{n_j} = U_{n_j}(theta) |0>^{q_n}                   (14.1)
    Psi_K = G_K ... G_2 G_1 Psi_0    (G_k applied in TEXT ORDER; gate list left to right, C4)                                (14.2)

Bending a pregroup sentence into a gate. The parse of Sec. 11.1 with the noun states removed is a frame f: n^{(x)k} ->
s, e.g. f[s; a, b] = V[a, s, b] for a transitive verb, a = subject (cups n^r), b = object (cups n^l). Two constructions:

(F) Frobenius bending (exact DisCoCat + copy maps): copy each noun wire with the spider, feed one copy to f, close the s
wire with a fixed effect e[s] (e = 2^{-q_s/2} (1, .., 1) = <+|^{q_s}, or learnt):

    G_F[a'_1..a'_k ; a_1..a_k] = prod_j delta(a'_j, a_j) v[a_1..a_k],   v[a_1..a_k] = sum_s e[s] f[s; a_1..a_k]              (14.3)

G_F is DIAGONAL: Psi[i] *= v[ (i >> q_n i_subj) & (d-1), (i >> q_n i_obj) & (d-1) ], 2^W complex products, v has 2^{2 q_n}
entries (4 / 16 complex). It is non-unitary (renormalise by Z = ||Psi||^2 in fp32, carry log Z) and, being diagonal, all
such gates COMMUTE: a text becomes a bag of sentences. Use (F) only for bag-of-facts tasks.

(U) Unitary bending (Coecke 2020 DisCoCirc; Duneau et al. 2024; UNVERIFIED gate set -- lambeq's experimental
DisCoCircReader, lambeq_src/lambeq/experimental/discocirc/reader.py, emits Frame boxes converted to "sandwich" boxes
and then whatever ansatz the user selects, so no DisCoCirc-specific gate set exists there; none of Sec. 14 depends on
it: any C10 ansatz on 2 q_n qubits is a valid (U) gate). The parse contributes only the WIRE ASSIGNMENT and ARGUMENT
ORDER (C17); the gate is

    G_V = U_ansatz(theta_V) in U(d^2),  local m = a + d b (a = subject, low; b = object, high), mapped by C17's wire_map        (14.4)

Sim15 is REAL (Ry + CNOT only, C10), so its G_V lies in SO(d^2): 6 parameters at d = 2, 120 at d = 4; it NEVER covers
SU(4) / SU(16), and since q_n = 1 nouns are complex (Rx Rz Rx) a real verb cannot even implement a controlled phase.
For a complex verb gate use Sim14 (CRx, ansatz_id 2, P = 8 q_n L) or interleave Rz layers; a full U(4) at q_n = 1
needs >= 15 angles (e.g. a KAK / Cartan form: Rz Ry Rz on each qubit, 3 two-qubit rotations, Rz Ry Rz on each --
define as ansatz_id 5 in C10 and the Sec. 6.3 header if used; UNVERIFIED which L saturates SO(d^2) for Sim15 or
SU(d^2) for Sim14). A real-only text model (Sim15 gates with ansatz-4 nouns) halves memory (real planes only, C3) and
is the recommended default when the readout is |amplitude|^2-based. Postselection disappears in (U): p_surv = 1, so
the Sec. 4.6 amplification factor is A = 2.

Gate-list construction from a parsed text (classical, once per text; C17 order):

    build_gates(text):
      wires = coref_resolve(text)                        // Sec. 14.5: token -> wire id; first mention creates a wire; pre-pass gives N
      for clause c in text order:
        (cups, S) = parse(words(c))                      // Sec. 11.1 chart parser
        for each verb v in c: subj(v) = head_noun(partner of v.n^r), obj(v) = head_noun(partner of v.n^l)   // C17: follow n^l cups
                                                          //   through adjectives / determiners ("big Bob": TV.n^l cups ADJ.n, ADJ.n^l cups Bob.n)
        emit (1) U_adj(theta) on field wire(j) for every adjective / determiner of an argument noun j, text order;
             (2) U_V on (wire(subj), wire(obj)) [transitive], U_IV on wire(subj) [intransitive]; coordination "j and k V m":
                 two gates sharing theta_V on (wire(j), wire(m)), then (wire(k), wire(m)) (text order of the conjuncts);
                 a relative clause "j who V k" is its own clause emitted BEFORE the main clause; pronouns / conjunctions emit no gate;
             (3) U_adv on the verb's fields (q_n or 2 q_n qubits), after the verb (G_adv G_V, C4)
      emit the gate list in the Sec. 2.6 program format extended with G2F(field_subj, field_obj, q_n, ansatz_id, param_offset)
      and G1F(field, q_n, ansatz_id, param_offset)  (Sec. 2.6 has no opcode for a runtime-field dense gate)

Register residency (C14): a text gate's fields are DATA, so register residency needs a switch over all C(N,2) compile-
time field pairs (15 cases at W = 6, 28 at W = 8, 45 at W = 10) with a uniform runtime selector; otherwise the state
lives in SMEM / VTCM. Memory of the exact text state, 2^{q_n N} amplitudes (fp32 complex 8 B / fp16-Q1.15 4 B; VTCM
entries assume a reservation of the stated bytes plus scratch):

    N   q_n=1 amps  fp32 / fp16   where                                            q_n=2 amps  fp32 / fp16   where
    2   4           32 / 16 B     CUDA thread regs (switch), HVX regs               16         128 / 64 B    thread regs / HVX regs
    3   8           64 / 32 B     same                                              64         512 / 256 B   thread regs / HVX regs (W = 6)
    4   16          128 / 64 B    same                                              256        2 KB / 1 KB   warp Layout B-blocked; HVX 16 / 8 vectors, tight
    5   32          256 / 128 B   same                                              1024       8 KB / 4 KB   warp B-blocked R = 32; VTCM slice (fp16); W = 10
    6   64          512 / 256 B   thread regs (128 fp32 regs); HVX 4 / 2 vectors    4096       32 KB / 16 KB SMEM (Layout C); Adreno local (fp32 exactly); VTCM >= 48 KB
    7   128         1 KB / 512 B  warp B-blocked R = 4; HVX 8 / 4 vectors           16384      128 KB / 64 KB dynamic SMEM (163 / 227 KB parts); VTCM >= 160 KB
    8   256         2 KB / 1 KB   warp B-blocked R = 8; HVX 16 / 8 vectors          65536      512 KB / 256 KB VTCM >= 512 KB (fp32) / >= 256 KB (fp16); no SMEM
    HVX vector counts are given as 32-bit lanes / 16-bit lanes. Adreno local 32 KB: exact to W = 12 fp32 / 13 fp16.
    Under the 8 KB VTCM slice the exact tier ends at W = 10 fp16 (W = 9 fp32).

Rule: registers to W = 6 (CUDA) or 7-8 (HVX); one warp to W = 10; SMEM / VTCM to W = 14-16; an 8 MB VTCM (v73+,
Sec. 3.2 confirmed) holds the exact state to W = 19 fp32 / 20 fp16. Beyond N = 8 at q_n = 2 (or N = 16 at q_n = 1) the
exact state exceeds every register file and every CUDA shared memory; that is where 14.3 takes over.

### 14.2 Recursion is depth, not width

A nested clause adds a gate on wires that already exist; a longer text adds gates. W = q_n N grows only with the
number of DISTINCT entities, so the memory of (14.2) is independent of text length and nesting depth and the cost is
the gate count:

    Ng_text = sum_clauses Ng(c);  Ng(c) = 1 fused 4x4 at q_n = 1 (all gates on one wire pair fuse, Sec. 2.2);
              Ng(c) ~ 8 L fused 4x4 at q_n = 2 (Sim15, Ry pairs fused into ring CNOTs)                                       (14.5)

Sec. 4.6 with A = 2 (no postselection) bounds the number of clauses before the output-probability error exceeds 0.01
(worst / typical; fp rows: a fused 4x4 counts 2.5, so the q_n = 2 column divides Ng_max by 16 x 2.5 = 40; fixed-point
rows count 1, divide by 16):

    format                              W     Ng_max        clauses, q_n = 1     clauses, q_n = 2, L = 2 (16 fused / clause)
    fp32 CUDA                           any   8333 / >1e6   3333 / >4e5          208 / >2.5e4
    fp16 store, fp32 compute            any   10 / 318      4 / 127              0 / 7
    Q1.15 int16, int32 accumulate       8     11 / 567      11 / 567             0 / 35
    Q1.15                               10    6 / 152       6 / 152              0 / 9
    Q1.15                               12    3 / 39        3 / 39               0 / 2
    Q1.15                               16    0 / 2         0 / 2                -

Consequence: on the NPU in Q1.15 a text of tens of clauses at q_n = 1 is inside the typical tolerance up to W = 10; at
q_n = 2 fixed point supports a handful of clauses and fp32 (GPU, or Q1.31 scalar) is required. Truncation errors of
14.3 add linearly to the same budget, E <= A (Ng g + sum_k eps_k), because every gate is unitary and every truncation
is a projection (norm <= 1 propagation, Sec. 4.1).

### 14.3 Bounded-memory contraction: the text state as an MPS over noun wires

Site tensors A_i[alpha][s][beta] (alpha < chi_{i-1}, s < d, beta < chi_i, chi_{-1} = chi_{N-1} = 1); bond dimensions are
PER BOND and variable:

    Psi[s_0 + d s_1 + ... + d^{N-1} s_{N-1}] = A_0[s_0] A_1[s_1] ... A_{N-1}[s_{N-1}]   (row x matrices x column)             (14.6)
    memory = sum_i d chi_{i-1} chi_i <= N d chi^2 complex,   chi_i <= min(d^{i+1}, d^{N-1-i}, chi)                           (14.7)

Psi_0 is a product state: chi = 1, A_i[0][s][0] = phi_{n_i}[s]. Keep the MPS in mixed-canonical form with the
orthogonality centre at the site being updated (left of it A^H A = I over (alpha, s); right of it B B^H = I over
(s, beta)); then the SVD truncation is the optimal rank-chi approximation of the whole state and its error is exactly
the discarded weight. A 2-wire gate G[m_out][m_in] (C6 local m = s + d t, s on site i, t on site i+1; if the SUBJECT
sits on site i+1 apply the C17 permutation pi to G first) is applied as

    Theta[alpha][s][t][beta]   = sum_gamma A_i[alpha][s][gamma] A_{i+1}[gamma][t][beta]              (d^2 chi^3 MACs)          (14.8)
    Theta'[alpha][s'][t'][beta] = sum_{s,t} G[s' + d t'][s + d t] Theta[alpha][s][t][beta]           (d^4 chi^2 MACs, in place)  (14.9)
    M[alpha d + s'][t' chi_{i+1} + beta] = Theta'  (a (d chi_{i-1}) x (d chi_{i+1}) matrix; M IS Theta' reshaped)
    M = U diag(S) V^H,  S SORTED descending (see the Jacobi recipe);  r = #{k : S_k > S_tol S_0}, S_tol = 1e-6 (fp32);
    chi_i := min(r, chi)                                                                                                  (14.10)
    A_i[alpha][s'][k] = U[alpha d + s'][k],  A_{i+1}[k][t'][beta] = S[k] conj(V[t' chi_{i+1} + beta][k]),  k < chi_i           (14.11)
    eps^2 = sum_{k >= chi_i} S_k^2 = ||Psi_trunc - Psi||^2;  ||Psi_trunc||^2 = 1 - eps^2  (centre now at i+1)                 (14.12)
    then EITHER scale A_{i+1} by 1/sqrt(1 - eps^2) (one scalar rsqrt, chi_i d chi_{i+1} multiplies) OR carry
    log Z_text = sum_k log(1 - eps_k^2) and divide every rho_q by Tr rho_q in 14.6 (Sec. 4.5: every output is a ratio)

The rank guard in (14.10) is load-bearing: S_k = 0 exactly whenever Theta' is rank-deficient with representable
entries (identity / diagonal / permutation gates on a product bond, Sim15 at theta = 0 on |0> nouns, SWAP of an
unentangled wire), and dividing by it propagates NaN through every later contraction (reproduced: verH/mps_nan.c
prints -nan with |0> nouns and an identity first gate). Columns k >= r are dropped as zero discarded weight; every
later loop bound uses chi_i, not chi. Moving the centre from i to i+1 without a gate: SVD (or, cheaper, Householder
QR) of A_i as a (d chi_{i-1}) x chi_i matrix, keep U as A_i, multiply diag(S) V^H into A_{i+1}: Jacobi ~16 d chi^3
FMA per sweep x 4-8 sweeps plus d chi^3 MACs for the product (chi = 16: 0.5-1 MFMA); leftwards likewise on A_i^H.
mpstest.c verified (14.8)-(14.12) against the exact statevector: chi = 8, N = 6, 30 random U(4) gates: ||Psi_mps -
Psi_exact||^2 = 4.5e-12; ./mpstest 2 11 (eleven gates, one truncation at chi = 2): error 1.379e-2 = discarded weight
1.379e-2 to all printed digits, ||Psi_trunc||^2 = 0.986212; chi = 4, 30 gates: error 1.04e-2 vs summed discarded
1.06e-2 (the sum is an estimate; the bound is (sum_k eps_k)^2).

Non-adjacent wires (i, j): either a swap network (SWAP as the 2-site gate (14.8)-(14.12), j-i-1 times towards i, the
real gate, j-i-1 swaps back: 2(j-i-1) + 1 SVDs; moving one d-dimensional site across a cut grows that cut's Schmidt
rank by at most a factor d, so it truncates like a real gate only when chi is already saturated), or, cheaper in bond
growth, reorder the wires PERMANENTLY so that frequently co-mentioned nouns are adjacent (14.4). A long-range MPO
(bond dimension d^2 carrying the gate across the interval) has the same asymptotic cost and is not needed at N <= 32.

Jacobi SVD, no LAPACK. One-sided (Hestenes) Jacobi on the m x n matrix M (m >= n; for m < n run it on M^H and then
A_i[alpha][s'][k] = V'[alpha d + s'][k], A_{i+1}[k][t'][beta] = S_k conj(U'[t' chi_{i+1} + beta][k]) with U' the
orthogonalised columns of M^H divided by S_k): sweep over column pairs (p, q), form the 2x2 Gram matrix, rotate both
columns of M and of V by the unitary that diagonalises it:

    alpha = ||m_p||^2,  beta = ||m_q||^2,  gamma = m_p^H m_q = |gamma| e^{i phi}       (8 m real FMA)                      (14.13)
    zeta = (beta - alpha) / (2 |gamma|),  t = sgn(zeta) / (|zeta| + sqrt(1 + zeta^2)),  c = 1 / sqrt(1 + t^2),  s = c t
    w = e^{-i phi} m_q;   m_p' = c m_p - s w;   m_q' = s m_p + c w     (same rotation on columns p, q of V)     (12 m + 12 n FMA)
    stop when |gamma| <= tol sqrt(alpha beta) for every pair in a sweep;  S_k = ||m_k||,  U[:, k] = m_k / S_k (S_k > 0 only)
    then SORT: selection sort on S descending, swapping the corresponding columns of M (= U S) and of V in lockstep
    (n^2 compare-swaps, m + n complex moves per swap); only after the sort apply (14.10)-(14.12)                          (14.14)

Without the sort the singular values come out in convergence order (verH/jsort.c: unsorted in 100 of 100 random 8x8
trials) and "keep k < chi" truncates arbitrary columns. Verified in jsvd.c: n = 32, fp32, tol 1e-7: 8 sweeps,
||U S V^H - M|| / ||M|| = 3.6e-6, ||V^H V - I|| = 1.9e-5; n = 16: 6 sweeps; n = 4: 4 sweeps (fp64, tol 1e-15: 5 sweeps on
the Sec. 18 test matrix). Cost per sweep n(n-1)/2 pairs x (8 + 12 + 12) n FMA = 16 n^3: 0.52 MFMA at n = 32, 8 sweeps =
4.2 MFMA per 2-site gate at d = 2, chi = 16, versus 0.08 MFMA for (14.8) + (14.9): the SVD dominates. Scratch = 2 (d chi)^2
complex + d chi reals (Theta' overwrites Theta and IS M): 16,512 B = 16.1 KB at chi = 16, 4,160 B = 4.1 KB at chi = 8,
1,056 B = 1.0 KB at chi = 4 (d = 2). On
CUDA one warp with one ROW of M and of V per lane (64 + 64 registers at chi = 16) needs a 5-step shuffle reduction per
pair for (14.13); the sort and the truncation must be a compare-exchange network with predicated swaps (constant
indices), never an indexed column move. On HVX: two planar fp16 columns per 128 B vector (re of columns c, c+1; C3's
split applies at the array level) or interleaved re/im with a vshuff per complex multiply -- choose one and record it;
(14.13) sums reduce with 5 vror + vadd in fp32; zeta, t, c, s, 1/S_k on the scalar core per pair (HVX has no vector
divide / sqrt); accuracy of an fp16-STORED Jacobi is UNVERIFIED -- run (14.13) in fp32 / int32.

Exact 2^{q_n N} versus MPS (d = 2, complex fp32; MPS worst case N d chi^2, capped by (14.7) in brackets):

    N    exact               chi = 4            chi = 8              chi = 16                 chi = 32
    8    256   (2 KB)        512  (4 KB)        1024  (8 KB)         4096  (32 KB)            16384 (128 KB)     exact wins
    16   65536 (512 KB)      1024 (8 KB)        2048  (16 KB)        8192 [4776] (64 [37] KB) 32768 (256 KB)
    32   4.3e9 (32 GB)       2048 (16 KB)       4096  (32 KB)        16384 [12968] (128 [101] KB)  65536 (512 KB)

Break-even N d chi^2 < 2^N: N >= 9 at chi = 4, N >= 11 at chi = 8, N >= 13 at chi = 16, N >= 15 at chi = 32. Below it
use 14.1 exactly; above it the MPS at chi = 16 keeps a 32-noun text in 101-128 KB plus 16 KB of scratch: dynamic SMEM
on cc 8.0 (163 KB) / 9.0-10.0 (227 KB) parts or a VTCM reservation >= 160 KB; there is no register tier for this
object (a warp holds at most 32 x 255 x 4 B = 32 KB). chi = 8: 27.3-32 KB (3,496 capped / 4,096 worst amplitudes at
N = 32) + 4.1 KB scratch (48 KB static SMEM; Adreno local only with fp16-stored site tensors and fp32 scratch). Under
the 8 KB VTCM slice: chi <= 4 (N = 32, capped: 936 amplitudes = 7,488 B = 7.3 KB fp32 / 3,744 B = 3.7 KB fp16 store,
+ 1.0 KB fp32 scratch), i.e. 2 log2 4 = 4 bits of correlation across any cut (14.4). Capped amplitude counts (14.7),
d = 2, for reference (critic/chk.c): N = 8: 168 / 424 / 680 / 680 at chi = 4 / 8 / 16 / 32 (the cap 2^{min(i+1, N-1-i)}
binds from chi = 16 on); N = 16: 424 / 1,448 / 4,776 / 15,016; N = 32: 936 / 3,496 / 12,968 / 47,784. A
1000-noun document costs 4 MB at chi = 16: out of every on-chip memory except an 8 MB VTCM.

### 14.4 Long-range dependencies: chi is the working memory

Across any cut between sites i and i+1, (14.6) is a Schmidt decomposition with at most chi_i terms:

    Psi = sum_{k < chi_i} S_k |L_k> (x) |R_k>,   S_cut = -sum S_k^2 log2 S_k^2 <= log2 chi                                   (14.15)
    I(a : b) <= I(L : R) = 2 S_cut <= 2 log2 chi   for any noun a left of the cut and b right of it                          (14.16)

Two nouns separated by a cut share at most 2 log2 chi bits of correlation, whatever the text between them. The exact
rank a text needs is bounded by the gates that straddle the cut (operator Schmidt rank <= d^2 per straddling 2-wire
gate; <= d per SWAP):

    chi_exact(i) <= min( d^{i+1}, d^{N-1-i}, d^{2 G_i + S_i} ),  G_i = 2-wire gates with one wire on each side, S_i = swaps crossing i;
    after swap-back the exact rank is bounded by d^{2 G_i} with G_i counting net gates only                                  (14.17)

Rule for chi: (1) order the wires to minimise max_i G_i (first-mention order, then greedy swaps of adjacent wires; a
cutwidth problem, NP-hard in general, N <= 32 makes greedy fine); (2) compute chi_exact = max_i (14.17); if it is within
budget the MPS is exact and needs SVDs but no truncation; (3) otherwise chi = budget and the discarded weights (14.12)
are logged per bond; a QA task that must recall k jointly correlated entities across a cut needs chi >= d^k (chi = 16 at
d = 2: four entities). Texts whose mention graph is local (characters introduced and dropped) are exactly
representable at small chi; a text where every sentence relates the first noun to a new one (a hub) has G_0 = number of
later sentences: put the hub in the MIDDLE of the chain (halves G_i) or accept truncation.

### 14.5 Anaphora and coreference

Which wire a pronoun refers to is a classical decision. Interface (C++; the resolver's quality is the dominant error
source of the pipeline; a rule-based resolver -- nearest antecedent with number / gender agreement, Hobbs-style -- is
UNVERIFIED on any benchmark; a neural resolver would itself be the transformer this design avoids, and lambeq's
DisCoCircReader indeed defaults to a neural one):

    struct Mention { int tok; int wire; float p; int alt_wire; float p_alt; };      // p + p_alt = 1; p_alt = 0 if certain
    int coref_resolve(const uint32_t* word_id, const uint8_t* lex_class, int n_tok, Mention* out, int max_wires);
    // returns N (wires numbered by first mention); a pronoun gets its antecedent's wire; -1 = unresolved

An ambiguous reference is a mixture of two gate placements (a quantum channel of Kraus rank 2, not a superposition):

    rho' = p G_{(i,k)} rho G_{(i,k)}^H + (1-p) G_{(j,k)} rho G_{(j,k)}^H                                                     (14.18)

A pure-state simulator cannot apply (14.18) in place: branch the text state (one extra 2^W vector, or one extra MPS,
per live branch) and, because every readout is linear in rho, mix only the reduced readout matrices rho_q = sum_r p_r
rho_q^{(r)} (one extra 2^{2 q_n} complex accumulator per queried wire). B unresolved pronouns give 2^B branches; keep a
beam of K branches (K x the state) or resolve the rest classically.

### 14.6 Question-answering readout

Reduced density matrix of the queried wire q (field start f = q_n q), C7 addressing:

    rho_q[a][a'] = sum_{r < 2^{W - q_n}} Psi[scatter(r, rest) | (a << f)] conj(Psi[scatter(r, rest) | (a' << f)])            (14.19)
    cost 2^{W - q_n} x d(d+1)/2 complex MACs (Hermitian);  memory d^2 complex = 4 (32 B) at q_n = 1, 16 (128 B) at q_n = 2

From the MPS with the centre at site q: rho_q[s][s'] = sum_{alpha, beta} A_q[alpha][s][beta] conj(A_q[alpha][s'][beta]),
d^2 chi^2 MACs, no scratch. Two wires q < q' (relational QA, "does Alice follow Bob"), centre at q, sites right of q'
right-normalised; result index C7: s (wire q) low, t (wire q') high; bond pairs written (b, b') = ket, bra:

    L[(b, b')][(s, s')]         = sum_a  A_q[a][s][b] conj( A_q[a][s'][b'] )                          d^2 chi^3 MACs; d^2 chi^2 complex
    per site j = q+1 .. q'-1:
      X[(c, b')][(s, s'), t]    = sum_b  L[(b, b')][(s, s')] A_j[b][t][c]                               d^3 chi^3 MACs; d^3 chi^2 complex scratch
      L[(c, c')][(s, s')]       = sum_{b', t}  X[(c, b')][(s, s'), t] conj( A_j[b'][t][c'] )            d^3 chi^3 MACs   (site j traced out;
                                  never form the chi^2 x chi^2 transfer matrix: d^2 chi^4 MACs and chi^4 complex)
    Y[(c, c')][(t, t')]         = sum_e  A_{q'}[c][t][e] conj( A_{q'}[c'][t'][e] )                     d^2 chi^3 MACs; reuses X's scratch
    rho_{qq'}[s + d t][s' + d t'] = sum_{c, c'}  L[(c, c')][(s, s')] Y[(c, c')][(t, t')]                d^4 chi^2 MACs; d^4 complex result   (14.19')

Total O((q'-q) d^3 chi^3) MACs; scratch d^2 chi^2 (L) + d^3 chi^2 (X, then Y) complex = 1,024 + 2,048 complex = 8 KB + 16 KB
at d = 2, chi = 16 (0.5 KB + 1 KB at chi = 4), plus the d^4 = 16-entry result (128 B). With the centre at q, the sites
left of q contract to the identity on a (that is why L starts at q with no left factor); if the centre is elsewhere, move
it first (Sec. 14.3, QR). Tr rho_{qq'} = ||Psi||^2 = prod_k (1 - eps_k^2) when truncations were not renormalised: divide
by it. Fidelity with a pure candidate |c> =
U_c|0> ("happy Alice" = U_adj U_Alice |0>): F = <c| rho_q |c>, d^2 MACs. Mixed candidate sigma (another text's reduced
state): Uhlmann F = (Tr sqrt( sqrt(rho) sigma sqrt(rho) ))^2 -- at q_n = 1 the closed form of Sec. 1.5; at q_n = 2 TWO
4x4 Hermitian Jacobi eigendecompositions (sqrt(rho), then the eigenvalues of sqrt(rho) sigma sqrt(rho); scratchpad
jacobi.c) plus two 4x4 products. Multiple-choice: p[c] = (F_c + eps) / sum_c' (F_c' + eps), loss -ln p[y], i.e. (1.7)
with F in place of |sigma|^2. Every rho_q is divided by Tr rho_q when truncations were not renormalised (14.12).

### 14.7 Generative use

The model defines p(answer | text) through (14.19), never p(sentence). Cheap: scoring / reranking a candidate sentence
(one gate application, 2^W d^2 MACs or one SVD update, per candidate); fill-the-slot classification, scoring every
vocabulary entry w by the fidelity of the resulting noun state with a target (V forward passes: W = 8, V = 1e4: 1e4 x 256
x 4 = 1e7 complex MACs = 4e7 FMA-class, ~0.2-0.5 ms on one SM at 64-128 FMA/clk, ~0.4-1 ms with the (14.19) readout per
candidate); an autoregressive factorisation p(w_k | w_<k, text) ~ exp(beta F_k) from the same fidelities. Not cheap or
not defined: free-running generation with no target state (no distribution over sentences exists) and the inverse
problem "find the sentence whose gate maps rho to rho_target" (non-convex over the angles of every candidate word
sequence, combinatorial in the length). Any generative claim beyond scoring is a contrastive classification over a
fixed vocabulary at V forward passes per token.

Ceiling, once: exact DisCoCirc is register / VTCM-resident to N = 6-8 nouns at q_n = 1 and N = 4-5 at q_n = 2 (8 at
q_n = 2 needs a >= 256 KB VTCM reservation); an MPS extends it to N = 32 in 28-128 KB with correlations across any cut
capped at 2 log2 chi bits, and to N = 32 at chi = 4 under an 8 KB slice; the per-clause error budget of Sec. 4.6 caps
fixed-point texts at a few clauses for q_n = 2 and tens to hundreds for q_n = 1; nothing in the model produces text.

---------------------------------------------------------------------------------------------------------------

## 15. Positioning against transformers, and an evaluation plan

Conventions C1-C17. arxiv.org, aclanthology.org and quantinuum.com were unreachable through this session's proxy;
figures marked UNVERIFIED come from search snippets or memory. The Lorenz et al. row was checked against the local
copy scratchpad/qnlp.txt.

### 15.1 What has actually been achieved

QNLP (DisCoCat / DisCoCirc), all on 2-class or QA toy tasks:

    work                          task / data                                size                   model               result
    Meichanetzidis et al. 2020    grammar-aware sentence classification      16 sentences           q_n = q_s = 1       proof of concept
     (arXiv 2012.03756)                                                      (UNVERIFIED)           (UNVERIFIED)
    Lorenz et al. 2021 (JAIR 76,  MC: food vs IT, CFG-generated, vocab 17    130 sentences (65/65)  q_n = q_s = 1, IQP  simulation, 500 SPSA it.: train / test ERROR
     2023; arXiv 2102.12846;      RP: subject vs object relative clause,     105 phrases, vocab     q_n = 1, q_s = 0    16.9 % / 20.2 %;  RP: 9.4 % / 27.7 %.
     verified vs qnlp.txt)        RELPRON subset                             115                    (n-wire readout)    ibmq hardware (100 / 130 it.): test error 16.7 %
                                                                                                                        (MC, F = 0.85), 32.3 % (RP, F = 0.75);
                                                                                                                        permutation test p <= 0.001 (MC), p <= 0.10 (RP)
    Duneau et al. 2024 (arXiv     QDisCoCirc QA on synthetic "who follows    exact contraction to   text-level, 108     compositional generalisation from <= 8-20 to
     2409.08777; H1-1 trapped     whom" stories (bAbI-style)                 20 nouns; hardware     logical qubits in   21-30 actors demonstrated on hardware; exact
     ions)                                                                   21-30 nouns            20 physical (reuse) accuracies and training-actor count UNVERIFIED
    Laakkonen et al. 2024         QA within QDisCoCirc                       --                     --                  BQP-hard; fault-tolerant algorithms only
     (arXiv 2408.06061)
    Comparative framework 2025    bAbI-derived compositionality tests        not stated in the      quantum and neural  quantum and neural DisCoCirc within 5 % on
     (arXiv 2507.02940)           (productivity, systematicity,              abstract               DisCoCirc           productivity / substitutivity, >= 10 % apart on
                                  substitutivity, overgeneralisation)                                                   systematicity (abstract; UNVERIFIED details)
    QFFN-BERT 2025 (arXiv         SST-2 (10 % few-shot), DBpedia; simulator  full sets, few-shot    PQC feed-forward    76.95 % SST-2 at 10 % data, 8 layers; up to
     2507.02364)                                                             SST-2                  inside BERT         102 % of the classical baseline with FFN params
                                                                                                                        cut > 99 %. NOT a compositional model; > 95 % of
                                                                                                                        parameters classical. (A second "QET" source
                                                                                                                        could not be recovered: omitted.)

No DisCoCat / DisCoCirc result exists on any GLUE-class task. The largest vocabularies in the compositional line are
~1e2 words. RP is the only experiment on real (RELPRON) text; its 27.7 % test error on a 2-class task with 105 items is
the honest state of the art for "real" sentences.

Classical tensor-network language models: Pestun & Vlassopoulos 2017 (arXiv 1710.10248): tree tensor-network LM,
algebro-geometric analysis, no benchmark numbers. Miller, Rabusseau, Terilla 2021 (AISTATS, arXiv 2003.01039): uniform
MPS (Born machine) vs 1-layer LSTM at matched hidden size (bond dimensions UNVERIFIED) on Tomita regular grammars and
CLAIR e-mail strings; character-level, synthetic; no PTB / WikiText, no transformer comparison. TTLM 2024 (arXiv
2405.04590): tensor-train LM on PTB and WikiText-2; beats VANILLA RNN perplexity at equal parameters (-14 to -16
perplexity for TTLM-Large, snippet), not compared with transformers. Tensor trains / MPOs as COMPRESSORS of
transformers: TT-embeddings (Khrulkov et al. 2019, arXiv 1901.10787) compress the embedding matrix with negligible
loss; MPOP (arXiv 2106.02205) keeps BERT GLUE scores with 91 % fewer fine-tuning parameters; rank-50 tensor-compressed
BERT training loses 1-2 % per GLUE task (arXiv 2306.01076). Reading: tensor networks are competitive when they
parametrise a transformer's WEIGHTS (nonlinearity and attention kept) and are 1-2 model generations behind when they
ARE the model. That is the gap the engineer names; the quantum-circuit model is a tensor network whose cores are
additionally constrained to be unitary images of |0>.

### 15.2 Why the gap exists, from first principles

(a) The DisCoCat sentence map is a fixed multilinear map. Given a parse, sigma[s] = sum_{k_1..k_C} prod_w phi_w[..];
each word OCCURRENCE appears in every monomial exactly once (a word repeated m times is degree m, Sec. 12.2). For
every occurrence w with the others fixed,

    sigma(phi_w) = A_w phi_w,   A_w a 2^{q_s} x 2^{Q_w} matrix built from the other words,   d^2 sigma / d phi_w^2 = 0       (15.1)

The class-k vs k' decision is sign( phi_w^H (A_k^H A_k - A_k'^H A_k') phi_w ), an indefinite Hermitian form: along any
affine line phi = u + t v it is a real quadratic in t with at most TWO sign changes; along a ray it is scale-invariant
and never changes; the decision boundary is a quadric cone. In theta-space (phi_w = U(theta_w)|0>) the decision is a
trigonometric polynomial of degree <= 2 per angle. And a word is a fixed point of a P_w-dimensional manifold, P_w in
1..4QL (C10): one meaning per parameter vector, no context (1.2).

(b) Attention is input-dependent routing: y_i = sum_j a_ij v_j, a_ij = exp(q_i . k_j / sqrt d) / sum_j' exp(..). The
weights are nonlinear in the SAME inputs the values come from; with all tokens but x_j fixed, y_i is not affine in x_j;
it can approximate argmax_j <q_i, k_j>: a selector.

(c) A fixed multilinear map, evaluated ONCE with each word entering once, cannot implement selection. Take two
candidates u, v and phi = (1-t) u + t v: affinity forces f(phi) = (1-t) f(u) + t f(v) -- the output MIXES, it does not
SWITCH. The Born readout p = |sigma[k]|^2 / Z is a RATIO of two quadratics in phi_w; on the segment it can cross 1/2 at
any t but its transition width is bounded below: p = 1 / (1 + c r^2) with r = (1-t)/t, so 10 %-to-90 % always spans a
factor 3 in r, whereas a softmax sharpens without bound as the logit scale grows. That is the exact statement that
distinguishes the readout from attention. Degree k in ONE argument requires that argument to enter k times. Note
what does and does not forbid it: no-cloning forbids copying an UNKNOWN state; a word state U_w(theta_w)|0> is
prepared from stored angles and can be RE-PREPARED k times (phi_w (x) phi_w is a tensor product, not a clone; on the
simulator it is a second read, Sec. 11.3 (11.8)). So the degree-1-per-word property is a property of the chosen
functor, not a law. Three legitimate escapes, each priced: (i) re-prepare a word k times (Frobenius copy written into
the grammar, or DisCoCirc threading a noun wire through k sentences): degree k in that word at cost k word
preparations (k x 2^{Q_w} generation ops), working set unchanged (each copy is contracted in turn); a degree-k
polynomial approximates a step to width ~1/k (Jackson / Chebyshev), so a switch of width delta costs k ~ 1/delta
copies -- and the OTHER words still act as fixed linear maps on those copies; (ii) enumerate candidates classically
(n parses (11.25), n antecedents (14.18), k senses (13.2c)) and take argmax over Z or p_K: a data-dependent routing
step costing n forward passes, parameters unchanged -- the same offloading already accepted for PP attachment;
(iii) a classical context front end (Sec. 13.2(b), 13.6) that performs the selection before the multilinear map.
What remains impossible: selection INSIDE a single evaluation of the fixed map, at any (q_n, q_s, L), because adding
parameters raises the dimension, not the degree.

(d) What real language needs. Agreement across an intervener ("the keys to the cabinet ARE") is solved by the GRAMMAR:
the preposition type n^r n n^l (Sec. 11.2; outside the Sec. 1.1 six shapes) routes "keys" to the verb regardless of
distance -- provided the parser is right, which is itself a selection problem (PP attachment, garden paths) pushed
outside the model. Coreference is the counter-example: "The trophy does not fit in the suitcase because IT is too
big / small" requires comparing candidate referents against the predicate and choosing: an argmax over n inner
products, impossible inside one evaluation by (c), available only via (ii) at n passes. Lexical ambiguity ("bank")
needs the word state to depend on context: impossible under (15.1)-(1.2) unless k senses are stored and something
SELECTS ((ii) or (iii)). Referent selection among 5-20 candidates and sense selection among 2-10 senses per word are
required for every sentence-pair benchmark in GLUE (MNLI, QQP, WNLI); a transformer supplies one such routing per
head per layer with 768-4096-dimensional states versus 2^{q_s} = 2-8 here.

(e) Parameter count and dimension. Sentence space is C^{2^{q_s}}: 2 complex numbers at q_s = 1; K classes need K <=
2^{q_s} (1.7). A word carries P_w real dof on a 2^{Q_w}-dimensional sphere; a BERT token carries 768 CONTEXTUAL reals.
The Sec. 7 budget (100 KB of uint16 angles = 5e4 angles) is 44x below bert-tiny (4.4 M parameters at int8 = 4.4 MB,
~83 % SST-2, UNVERIFIED), the smallest transformer that clears 80 % on SST-2. The budget at which the single-evaluation
compositional model could match: none, by (c); with escapes (ii)-(iii) the budget is the encoder's (Sec. 13.6).

### 15.3 Where the model class is the right tool

(a) Closed-domain command / intent grammars on MCU / NPU. The domain is a finite grammar the parser covers 100 %; no
coreference, no ambiguity; exact probabilities with deterministic latency. Configuration: q_n = 1, q_s = ceil(log2 K)
(K = 8 -> q_s = 3; K = 32 -> q_s = 5), IQP ansatz_id 1 with L = 2 (P: N 3, ADJ 2, IV 6, TV 8, ADV 10; mean ~5 at a
70 %-noun lexicon) or Sim15 L = 2 with Q = 1 words under ansatz_id 4 and flags.bit2 = 1 (real; P: N 1, ADJ 8, IV 16,
TV 20, ADV 24; mean ~7-9); lexicon V = 100-500 words. Memory: model V x mean(P_w) x 2 B = 1-9 KB; working set at (1,3)
2^5 + 2 + 2^4 + 2 = 52 amplitudes = 416 B fp32 / 208 B Q1.15 (34 in place: 272 / 136 B; real model halves these); at
(1,5) 196 amplitudes = 1,568 B fp32 / 784 B Q1.15 (130 in place: 1,040 / 520 B; above 255 registers per thread ->
CUDA Layout B/C); per-gate scratch <= 128 B; no unitary, no embedding table. Latency is a fixed, data-independent op
count: N TV N at (1,3): fold 1 = 2^{q_n} x 2^{q_n + q_s} = 32 complex MACs (128 FMA-class), fold 2 = 2^{q_s} x 2^{q_n} = 16
(64); word generation dominates: verb Q = 5, IQP ansatz 1, L = 2: (L+1) Q = 15 H gates and L (Q-1) = 8 CRz on 16 pairs
each ~ 1.2-2.4 K FMA-class, two nouns ~24 each: total ~1.5-2.7 K FMA-class per sentence. Honest comparator: a 1-layer
GRU with hidden width 8-16 int8 has a run-time state of 16-32 B, is order-sensitive and branch-free, with 384 B of
recurrent weights plus a V x 8 embedding table (4 KB at V = 500) -- the same parameter class. What is UNIQUE to the
model is not the state byte count but: no embedding / weight table at all (2 B per angle, streamed); exact Born
probabilities with a proven fp32 bound (Sec. 4.7); exact per-word adjoint attribution (Sec. 5.3); compositional
(multilinear) generalisation by construction.

(b) Compositional generalisation. Evidence that UNMODIFIED transformers fail: COGS (Kim & Linzen 2020, arXiv
2010.05465): in-distribution 96-99 %, generalisation split 16-35 %, +-6-8 % across seeds; SCAN length split: vanilla
transformer 0-10 %, best LSTM ~14 % (Lake & Baroni 2018, arXiv 1711.00350). With relative positional embeddings a
transformer reaches 100 % on SCAN length and 81 % on COGS (Csordas et al. 2021, arXiv 2108.12284), and auxiliary
sequence-prediction tasks also give 100 % on Length / MCD (Jiang & Bansal 2021, arXiv 2109.15256): the compositional
advantage of the multilinear model is over unmodified small transformers only (all figures UNVERIFIED, snippet-
sourced). The multilinear model is compositional BY CONSTRUCTION: every (noun, verb, noun) triple whose words were
each trained in any context gives a defined sigma; the productivity result of Duneau et al. is its quantum-circuit
instance. Configuration as (a) with Sim15 L = 2 (magnitude variation; ansatz 0 L = 1 has constant-modulus amplitudes,
C10); q_n = 2 with q_s = 3 gives a working set of 168 amplitudes = 1,344 B fp32 / 672 B Q1.15 (132 in place) and verb
Q = 7 (~9-14 K FMA-class generation); the noun-inventory threshold at which q_n = 2 is needed is UNVERIFIED (nothing
in the spec bounds it; a q_n = 1 noun is a Bloch-sphere point; determine it on the role-swap split of 15.4). Caveat:
the 2025 comparative study reports quantum and neural DisCoCirc within 5 % on productivity / substitutivity -- the win
is structure-vs-no-structure, not quantum-vs-classical.

(c) Interpretable, auditable meaning. Every intermediate object is a state with a metric: Fid = |<sigma_1|sigma_2>|^2 /
(Z_1 Z_2) (Sec. 1.5) is an exact fidelity; p[k] is an exact Born probability computed in fp32 with the tensordot bound
of Sec. 4.7 (Sec. 4.6 applies to the bent-wire circuit form); the per-word attribution is the exact adjoint gradient
(Sec. 5.3); inference has no temperature, dropout or nondeterministic kernel (training uses the segmented reduction
of Sec. 13.4 / 5.5 for bitwise reproducibility). The audit object for word w is A_w (15.1), 2^{q_s} x 2^{Q_w} (8 x 32 =
256 complex = 2 KB fp32 for the verb at (1,3)): column j = the tensordot of Sec. 1.3 with phi_w := e_j (2^{Q_w} passes),
computed off-device at audit time. Use where a regulator must be shown WHY "stop pump" was classified as it was.

(d) Compositional head over a frozen classical encoder (Sec. 13.6). The encoder does the selection of 15.2(c); the head
supplies structure and the metric. theta_w = M_type e_w, M_type in R^{P_type x d}, P_type from C10 for the class's Q
(at (1,3), IQP L = 2: 3, 2, 6, 8, 10); bytes = 2 d sum_classes P_type = 44,544 B at d = 768 (int16 with a per-row fp32
scale s_r: theta_i = s_r sum_j M_q[i][j] e_w[j] in fp32, then (13.2)); content-word angles come from M_type, function
words / OOV keep the (a) angle table; training dL/dM_type[i][j] = sign_k grad_local(theta_i) e_w[j] (Sec. 5.3 / 5.5).
The encoder is an external C++ inference runtime (a ggml-class BERT port), MB-scale, not part of this spec. This is
the configuration that could plausibly reach transformer-class accuracy on sentence-pair tasks; UNVERIFIED that it
beats a linear probe on the same encoder.

(e) Structured retrieval by inner product. Sentence embedding = 2^{q_s} complex numbers, comparable by Fid; two
sentences with the same words in different roles get different vectors. Dimension is the limit: at (2,6) the verb is
1024 amplitudes but the working set is 2^10 + 4 + 2^8 + 4 = 1,288 amplitudes = 10.3 KB fp32 / 5.2 KB Q1.15 (1,028 in
place: 8.2 / 4.1 KB): CUDA Layout B/C with an S(10) = 8 KB exchange buffer, Adreno local, VTCM (5.2 KB Q1.15 fits the
8 KB slice, fp32 does not); on HVX it exceeds the 4 KB register file. At (2,8): 5,128 amplitudes = 41 KB fp32 / 20.5 KB
Q1.15: SMEM or a VTCM reservation only; exceeds Adreno local in fp32 and the 8 KB slice in both formats. The model is
NOT unchanged: P_TV grows linearly in Q = 2 q_n + q_s (IQP L = 2: 2(Q-1) = 18 at (2,6), 22 at (2,8), vs 8 at (1,3);
Sim15 L = 2: 4Q = 40 / 48 vs 20). For corpora above ~1e4 items, 64-256 complex dimensions are below what dense
retrievers use (768); expect recall to saturate (quantify: UNVERIFIED).

### 15.4 Evaluation plan (replace GLUE)

1. Closed-domain intent set: 8-32 intents (K = 32 -> q_s = 5, working set 196 amplitudes), 200-500-word lexicon, 1-3 K
   sentences generated from a CFG that the Sec. 11 grammar covers (productions: S -> NP VP; NP -> N | ADJ N | N PP;
   VP -> IV | IV ADV | TV NP | TV NP PP; PP -> P NP); report accuracy, coverage (parse-FAIL rate = #FAIL / #held-out
   natural paraphrases through parse() of Sec. 11.6), bytes of state, ops per sentence.
2. Compositional splits on that set: (i) structure-swap: hold out every sentence containing a given (subject, verb,
   object) TRIPLE while every word appears in training (COGS "novel combination"); (ii) role-swap: test "A verb B" when
   only "B verb A" was seen; (iii) length / productivity: train ADJ-free, test ADJ N TV ADJ N; train stories with <= 4
   actors, test 8-16 (requires the DisCoCirc kernel of Sec. 14). Report generalisation AND in-distribution accuracy,
   5 seeds.
3. Public micro-benchmarks for comparability: MC (130) from Lorenz et al. -- fully covered by the Sec. 1.1 six shapes
   (CFG: NP -> N | ADJ N, VP -> TV NP, verified in qnlp.txt); RP (105) -- REQUIRES Sec. 11: relative-pronoun types
   n^r n s^l n and n^r n n^{ll} s^l (Lorenz et al. model "that" as a parameter-free GHZ / Frobenius state, i.e. the
   spider of (11.10)-(11.11)), the chart parser, and readout on the surviving n wire (target N in C15; p[k] =
   |sigma_n[k]|^2 / Z over 2^{q_n} outputs; q_s = 0 is a width-0 field: skip the cup). The bAbI-derived QDisCoCirc QA
   set (Sec. 14 kernel). RELPRON is a term-definition RANKING task ("device that detects planets" -> telescope): rank
   terms by Fid against the definition state; for ENTAILMENT (directional) do not use the symmetric Fid: use the
   density-matrix model (Sec. 1.6 / 13.2(c)) with an asymmetric score, k_hyp(rho_A, rho_B) = Tr(rho_A rho_B) /
   Tr(rho_A^2) or the Loewner test rho_B - rho_A >= 0 (Bankova et al., Lewis; UNVERIFIED citations). A 100-item
   lexical-ambiguity set ("bank", "bat") where the expected single-evaluation result is chance (predict it; do not hide
   it; then show 13.2(c) with K = 2 senses + argmax).
4. Matched-memory baselines, all trained in fp32 C++ (own autograd: embedding, linear, softmax attention, LayerNorm,
   GRU cell, cross-entropy; Adam per (5.9)), exported to int8 with per-tensor symmetric scale s = max|w| / 127,
   w_q = round(w / s); vocabulary hash = FNV-1a(word) mod V; each model reported at ITS OWN parameter bytes AND at a
   common 100 KB, with V matched across models: (B1) bag-of-embeddings + linear head: V d + d K bytes, V = 1e4, d = 8 ->
   80 KB; order-INVARIANT, so exactly 50 % on the role-swap split -- the control that shows structure matters. (B2) tiny
   transformer: V d + L (4 d^2 + 2 d d_ff) + d K; V = 1000 (hashed), d = 48, L = 2, d_ff = 128: 91 KB; working memory
   n x d int8 activations + n x d_ff FFN intermediate + n x n attention per head (n = 16: 768 + 2048 + 256 B ~ 3-4 KB),
   i.e. ~8-15x the 208-416 B QNLP state, with a data-dependent op count. (B3) 1-layer GRU, d = 64: 3 (64^2 + 64^2) =
   24.6 KB + embeddings; and the d = 8 GRU of 15.3(a) as the small-state comparator.
5. Expected outcomes (stated before running; UNVERIFIED until run): in-distribution intent accuracy B2 >= QNLP >= B3 >
   B1 within a few points; role-swap: QNLP ~ in-distribution, B1 = 50 %, B2 drops (COGS-type, 20-60 points at 1-3 K
   training sentences); length / productivity: QNLP holds where the shapes are covered, B2 degrades with length
   (SCAN-type); ambiguity set: QNLP at chance in single evaluation, B2 above chance only with pretraining, which the
   budget forbids; coverage: QNLP parse-FAIL on 10-30 % of free paraphrases with the Sec. 11 lexicon and a rule
   tagger, B1-B3 never fail. Report all of it.

### 15.5 Two statements for stakeholders

"At this memory budget this is what the model uniquely offers": at <= 1 KB of run-time state (K <= 8 intents in fp32,
416 B; K <= 32 in Q1.15, 784 B) and <= 10 KB of parameters (V <= 500, mean P_w <= 10), with NO weight or embedding
table, a deterministic op count and exact Born-rule probabilities with a proven error bound, the model classifies
grammatical, order-sensitive sentences from a closed grammar of 1e2-1e3 words and generalises to unseen word
combinations and to longer sentences of covered shapes; a bag model at that budget is order-blind, a GRU at that
budget has a smaller state but a weight table and no exact attribution, and an unmodified transformer at that budget
has no room for pretraining and fails compositional splits (COGS 16-35 %, SCAN length 0-10 %; UNVERIFIED figures).

"This will not do Y; for Y use Z": in a single evaluation of the fixed multilinear map it will not resolve
coreference, lexical ambiguity, open-vocabulary input or anything in GLUE / MNLI-class benchmarks, at any (q_n, q_s,
L) budget, because the map is affine in each word occurrence and cannot select (15.2(c)); it CAN do them only by
paying n forward passes per selection (candidate enumeration), by k re-preparations per word (degree k), or by a
classical encoder in front (MB-GB scale) with this model as a compositional head (15.3(d)) when a metric and an audit
trail are required.

Sources cited above: https://arxiv.org/abs/2102.12846 (Lorenz et al.; numbers from the local copy qnlp.txt),
2012.03756, 2409.08777, 2408.06061, 2507.02940, 2507.02364, 1710.10248, 2003.01039, 2405.04590, 1901.10787,
2106.02205, 2306.01076, 2010.05465 (COGS), 1711.00350 (SCAN), 2108.12284, 2109.15256.

---------------------------------------------------------------------------------------------------------------

## 16. Updated memory budget table (extends Sec. 7)

Bytes per amplitude: fp32 complex 8, fp16 / Q1.15 complex 4, real ansatz halves both. Tiers per C14: REG = one CUDA
thread (~1 KB) or the HVX register file (4 KB); VTCM = 8 KB slice unless "VTCM >= X"; SMEM = CUDA shared 48-227 KB /
Adreno local 32 KB; HBM = global / L2 / DDR. "Precision" notes the Sec. 12.3 rule (fp16 / Q1.15 storage generically
valid for q_n <= 4 only).

    Object (kernel, eq.)                                 formula                           (1,1) fp32 / fp16-Q15     (2,1) fp32 / fp16-Q15    tier (1,1) -> (2,1)
    --- Sec. 7 rows unchanged (model 100 KB, tensordot 16 (10) / 48 (36) amps, bent-wire, training 24 x 2^W, ...) ---
    relative clause, subj. or obj., tensordot (11.14)    2^{2qn+qs} + 2^{qn} + 2^{qn+qs} (slot: 2^{2qn+qs} + 2^{qn})
                                                                                           14 (10) amps: 112 (80) B / 56 (40) B   44 (36): 352 (288) B / 176 (144) B   REG -> REG (same as N TV N)
    relative clause, circuit form W_hw (11.23)           2 q_n + q_s                       8 amps: 64 B / 32 B       32 amps: 256 B / 128 B   REG
    relative clause, training on the circuit (11.23')    24 x 2^{3 q_n + 2 q_s}            768 B                     6 KB                     REG -> SMEM/VTCM >= 8 KB  (not the training path)
    training, tree adjoint (11.31), ANY sentence         sum_w 2^{q_out} + 3 x 2^{Q_max} + peak(11.18')  (fp32 only, Sec. 5.3)
                                                                                           T12 sentence, 5 words: 50 amps = 400 B;  20 words at (2,1): 220 amps = 1.7 KB   REG / VTCM
    training on the circuit, 20-word sentence (11.23'')  24 x 2^{W_train}, W_train ~ 25-30 at (2,1)                       0.75-24 GB              HBM (host) -- do not use
    relative-clause tensors (who, whom)                  0 (index equality, C15)           0                         0                        --
    noun / VP coordination, tensordot (11.16)/(11.17)    2^{qn} + 2^{qn+qs}                6 amps: 48 B / 24 B       12: 96 B / 48 B          REG
    sentence coordination, merge step (11.13')/(11.17')  3 x 2^{qs} (2 x 2^{qs} in place)  6 (4) amps: 48 B / 24 B   6 (4): 48 B / 24 B       REG (plus the larger sub-sentence's own peak, held: + 2^{qs})
    coordination, circuit form (Delta ancilla)           2 q_n + q_s                       8 amps: 64 B / 32 B       32: 256 B / 128 B        REG
    sentence coordination, circuit form (both islands live)  W_1 + W_2                     N TV N pair: 6 qubits, 64 amps: 512 B / 256 B   10 qubits, 1024 amps: 8 KB / 4 KB   REG -> VTCM / SMEM
    tree[] + cups[] per sentence (C16)                   14 B x words + 2 B x cups          5 words: 70 + 10 = 80 B     20 words, 24 cups: 328 B   REG / VTCM (device input)
    sentential complement, tensordot ("A believes B sleeps")  2^{qn+2qs} + 2^{qn} + 2 x 2^{qs}   14 amps: 112 B / 56 B    24: 192 B / 96 B         REG
      (with a TV inside the complement: dominated by the TV, 14 (10) / 44 (36) as N TV N)
    sentential complement, circuit island-first (11.23)  q_n + 3 q_s                       16 amps: 128 B / 64 B     32: 256 B / 128 B        REG
    sentential complement, training W_train              24 x 2^{3 q_n + 3 q_s}            1.5 KB                    12 KB                    REG(HVX) -> SMEM/VTCM >= 16 KB
    4-cycle through a unitary wrapper (11.18'')          2^{Q_wrap} + 2^{q_a+q_b} + 2^{Q_wrap - q_a - q_b}   24 amps: 192 B / 96 B     80: 640 B / 320 B        REG
                                                         (n^r s s^l n wrapper: 16 + 4 + 4 / 64 + 8 + 8; equals 2^{Q_wrap} + 2 x 2^{q_a+q_b} only when Q_wrap = 2 (q_a + q_b))
    held-sibling worst case, two relative clauses        2^{2qn+qs} + 2^{qn} + 2^{qn+qs} + 2^{qn}   16 (12) amps: 128 B / 64 B   48 (40): 384 B / 192 B   REG
    chart parser (host, per sentence), m = 92 / 256      m^2/8 + m^2 + m^2/4 = 1.375 m^2 B  11.4 KB / 88 KB (11,638 / 90,112 B; host or DSP scalar; NOT the device budget)   host
    ambiguity accumulators (11.25)/(11.26)               2^{qs} / 2^{2qs} complex          16 B / 32 B               same                     REG
    density-matrix verb (13.10)                          2^{2(2qn+qs)} complex             64 amps: 512 B / 256 B    1024: 8 KB / 4 KB        REG -> VTCM (Q15) / SMEM
    density-matrix working set (13.10)                   rho1 + rhoV + R + rho2            88 amps: 704 B / 352 B    1120: 8.75 KB / 4.4 KB   REG(HVX amp-per-lane; CUDA warp) -> SMEM / VTCM (Q15 only)
    MPO verb r = 4, stored cores (12.13), d = 2^{qn}, d_s = 2^{qs}   d r + r^2 d_s + r d   at (4,2): 64 + 64 + 64 = 192 amps: 1.5 KB / 768 B; at (8,2): 1024 + 64 + 1024 = 2112: 16.5 KB / 8.25 KB   REG / VTCM -> SMEM
    MPO verb r = 8, stored cores                          same                              at (4,2): 128 + 256 + 128 = 512: 4 KB / 2 KB; at (8,2): 2048 + 256 + 2048 = 4352: 34 KB / 17 KB   VTCM -> SMEM
    MPO verb, resident when cores streamed (12.14)       d + 2 r + d_s (+1)                 r = 4, (4,2): 28 amps: 224 B;  r = 8, (8,2): 276 amps: 2.2 KB (fp32 required at q_n >= 5)   REG -> VTCM (fp32)
    MPO verb, angle-parameterised (12.15), q_r = 3       2^{qn} + r^2 d_s + 2 r + d_s        (8,2): 532 amps: 4.2 KB fp32 (Q15 not valid at q_n = 8)   VTCM (fp32)
    DisCoCirc exact text, N = 4 nouns (14.1)             2^{N qn}                           16 amps: 128 B / 64 B     256: 2 KB / 1 KB         REG -> REG (warp B-blocked / HVX 16 vec) / VTCM
    DisCoCirc exact text, N = 8                          2^{N qn}                           256 amps: 2 KB / 1 KB     65536: 512 KB / 256 KB   REG (warp / HVX tight) / VTCM -> VTCM >= 512 KB (fp32) or >= 256 KB (fp16); HBM otherwise
    DisCoCirc transient dense verb                       2^{4 qn}                           16 amps: 128 B / 64 B     256: 2 KB / 1 KB         REG
    DisCoCirc mean-field, per noun                       2^{2 qn} complex                   32 B / 16 B               128 B / 64 B             REG
    MPS text, N = 16, chi = 16, d = 2 (14.7)             sum_i d chi_{i-1} chi_i <= N d chi^2   4776 [worst 8192] amps: 37.3 [64] KB / 18.7 [32] KB  (q_n = 1 only)   SMEM (48 KB static fp32 needs the capped figure) / VTCM >= 64 KB
    MPS SVD scratch, chi = 16 / 8 / 4                    2 (d chi)^2 complex + d chi reals  16,512 / 4,160 / 1,056 B = 16.1 / 4.1 / 1.0 KB fp32   SMEM / VTCM >= 24 KB (chi = 16)
    MPS text, N = 32, chi = 4 (8 KB-slice regime)        capped (14.7): 936 amps            7,488 B = 7.3 KB fp32 / 3,744 B = 3.7 KB fp16 + 1.0 KB fp32 scratch   VTCM slice (fp16 store, fp32 scratch)
    MPS two-wire readout scratch (14.19')                d^2 chi^2 + d^3 chi^2 complex      chi = 16: 8 KB + 16 KB;  chi = 4: 0.5 KB + 1 KB        SMEM / VTCM (chi = 16); VTCM slice (chi = 4)
    hybrid model file, generator block (13.7)            16 + 32 nclass + nclass m_in + 4 + sum_c P_c (m_in + 4) [+ B m] [+ ctx]
                                                                                           (1,1) IQP L=2, m=32, zero-table: 808 B; + learned E 2^15 x 32: 1 MB; (5,2) Sim14 L=2, m=384: 117.2 KB   REG / VTCM / SMEM (as the generator f row)
    hashed n-gram front end, learned table (13.4)        B m int8                           B = 2^13, m = 16: 128 KB;  B = 2^15, m = 32: 1 MB   SMEM (128 KB, cc 8.0+) / VTCM >= 1 MB / HBM
    hashed n-gram front end, zero table (13.4')          0 B (+ f: sum_c P_c (m+1) int8)    0 B + 429 B at (1,1), m = 32                        REG
    angle generator f (13.3), m = 32 / d = 384           sum_c P_c (m+1) int8               (1,1): 429 B / 5 KB;  (5,2) Sim14 L=2: 10 KB / 117 KB   REG -> VTCM / SMEM
    context gate (13.9), m = 64                          2 m^2 int8 (x2 bidirectional)      8 KB / 16 KB                                        VTCM (>= 16 KB) / SMEM
    int8 embedding table (13.1 b)                        V m + 2 V + 6 V / 0.75             V = 50 K, m = 64: 3.5 MB                             HBM (L2 / VTCM >= 4 MB)
    POS tagger, averaged perceptron (11.6)               ~0.5-4 MB fp16 (UNVERIFIED)        0.5-4 MB                                             HBM
    frozen int8 encoder (13.6)                           model size                         MiniLM-L6 class ~22 MB; BERT-base ~110 MB (UNVERIFIED) HBM (order of magnitude: 1e7-1e8 B)
    compositional head on the encoder (13.6)             sum_c P_c (d+1) int8               (1,1): 5 KB;  (5,2): 117 KB                          VTCM >= 8 KB / SMEM
    training, hybrid: Adam state                         16 B/param                         f: 6.9 KB; E (2^15 x 32): 16 MB; context: 128 KB      HBM (E), SMEM (f)
    training, hybrid: recurrent hidden states            T m fp32 per direction             n_w = 8, m = 64: 2 KB                                 SMEM

Reading of the table. Every SENTENCE-level construction of Sec. 11 stays in the register tier at (1,1) and (2,1): the
widest word sets the peak (11.18'), and relative clauses, coordination and complements add at most a few 2^{q}
vectors. Training by the tree adjoint (11.31) stays in the register / VTCM tier for EVERY sentence (400 B - 1.7 KB); only
training on the circuit grows with nesting and leaves on-chip memory at a 20-word sentence (11.23''). The first objects
that leave the register tier are the training circuit for a clause pair at (2,1) (6-12 KB, avoidable), the
density-matrix verb at (2,1) (8.75 KB fp32), an MPO verb at q_n = 8 with stored cores (16-34 KB), a DisCoCirc text
of 8 nouns at q_n = 2 (512 KB) and any MPS at chi >= 8. Everything in the hybrid front end except the zero-table
variant (0 B) and the generator f (< 1 KB at (1,1)) is off-chip: 128 KB - 1 MB for learned character features, 3.5 MB
for pretrained vectors, 0.5-4 MB for a tagger, 20-110 MB for an encoder -- the memory of the SYSTEM is then the front
end's, and the "almost zero" claim applies to the head alone.

---------------------------------------------------------------------------------------------------------------

## 17. Direct answers to the two criticisms

(1) "The lexicon has six shapes, no relative clauses, coordination, long-range dependencies or recursion beyond simple
adjoints." Lifted: Sec. 11 gives the free-pregroup recogniser (a chart parser, O(m^3), 11.4 KB of host memory for a
40-word sentence; the greedy reducer is demoted to a six-shape fast path), a lexicon of about 20 further classes with
verified cups (ditransitives, adverbs of three kinds, prepositions, auxiliaries, negation, sentential complements,
complementisers, noun / VP / sentence coordination, subject and object relative pronouns, possessives, object-gap
questions), Frobenius spiders for relative pronouns and coordination at zero stored parameters (C15), unbounded-
distance dependencies through iterated adjoints, right-embedding recursion at constant depth, and a contraction order
(11.18') for arbitrary word graphs including the cycles that post-verbal modifiers create, as an explicit procedure
(build_tree / eval_tree / spider / chain pseudocode, Sec. 11.4); about 25 node-op kernel instantiations cover it all
(C16), the fused tensordot itself is unchanged, and training is exact reverse mode on the same tree (Sec. 11.7) with
memory independent of the sentence. Exact memory price: a relative clause costs what its verb costs (14 / 44
amplitudes = 112 / 352 B fp32 at (1,1) / (2,1), 10 / 36 in place), coordination 6 / 12 amplitudes (sentence
coordination 6 at any q_n), a sentential complement 14 / 24, a 4-cycle 24 / 80, a held sibling + 2^{q_n}; the parser
adds 11-88 KB on the host; training costs 400 B (T12 sentence) to 1.7 KB (20 words at (2,1)) by (11.31); only the
circuit form grows with nesting (24 x 2^{W_train}: 768 B / 6 KB for a relative clause, 1.5 / 12 KB for a complement,
GB for 20 words) and it is not the training path. What remains: cross-serial dependencies have no planar
type (take cups from a dependency parser; the kernel does not care); ellipsis is a basis copy only; anaphora and
cross-sentence phenomena need DisCoCirc (Sec. 14), where the price is 2^{q_n E_live} amplitudes (2 KB at 8 entities,
q_n = 1; 512 KB at 8 entities, q_n = 2) or an MPS at N d chi^2 (28-128 KB for 32 entities) with correlations across a
cut capped at 2 log2 chi bits; and every word must be typed, so an open-domain system needs a POS tagger of
0.5-4 MB that dwarfs the 100 KB model.

(2) "Word states come from a handful of angles; sentence meaning is a 2^{q_s}-dimensional vector; it will not compete
with transformers on real language tasks." Lifted: the angle count is not the bottleneck at (1,1) -- Sim14 / Sim15
saturate every Q <= 3 word manifold with <= 24 angles (Sec. 12.1); the bottleneck is d = 2^{q_n} reals per noun and
the function class. Capacity scales as 2^{k q} in memory for 2^{k q + 1} - 2 dof (12.20): a VTCM slice reaches (4,2)
(d = 16, 4 classes, 4 KB Q1.15, the precision limit of fixed point), SMEM reaches (5,2)-(6,2) (33.5-131 KB fp32), and
low-rank MPO verbs reach d = 256-1024 at r = 8-16 in 35-270 KB (12.13)-(12.15) with the same bilinear semantics; a
classical front end generates the angles from features (0 B zero-table to 1 MB learned character features, 3.5 MB
pretrained vectors, 20-110 MB frozen encoder), giving open vocabulary and, through a recurrent gate (8-16 KB) or the
encoder, context; ambiguity is a mixture at 5.5x / 23x the working set or K^{n_w} pure runs; sentence width can be
raised to q_s = 6-9 (working set 10 KB - 4 MB) or replaced by the DisCoCirc noun register (2 KB at 8 nouns). What
remains, and why: a single evaluation of the fixed multilinear map is affine in every word occurrence and its Born
readout is a ratio of quadratics with a bounded transition width, so it cannot select among referents or senses
(15.2(c)); selection costs n forward passes (candidate enumeration), k re-preparations (degree k), or a classical
encoder in front -- and then the capacity is the encoder's. The exact price of competing on real benchmarks is
therefore 20-110 MB of encoder plus a 5-117 KB head; without it the honest targets are closed-domain, order-sensitive
classification at 208-416 B of state and 1-10 KB of parameters, compositional generalisation splits where unmodified
small transformers fail, and auditable meaning with exact probabilities (15.3, 15.5). The "almost zero memory" claim
is true of the per-sentence working set (the widest word: 40-400 B) and of the parameter table only up to ~1e5 angles;
above that, and for every front end, it is false and the budget is stated in Sec. 16.

---------------------------------------------------------------------------------------------------------------

## 18. Extended implementation checklist and tests

Kernels / modules added to Sec. 9 (one compile-time instantiation per node op, C16):
12. Chart parser (11.4)-(11.5) with saturating counts, first-parse and k-th-parse extraction, accept(p, target),
    accept_prefix; host / DSP scalar. Greedy pre-check retained for the six shapes only.
13. build_tree (pseudocode Sec. 11.4): word graph from cups (parallel cups merged into one edge), DFS spanning tree,
    back-edge cycle detection with the ancestor as wrapper, chain nodes (Node.kind = 2), post-order with children sorted
    by decreasing peak (11.18'), assert nchild <= 4 and no intra-word cup; emits Node[] (C16, 14 B each).
14. Node ops (eval_tree, Sec. 11.4): fused tensordot per (type string, cmask, pmask); the general spider loop (one index
    per basic type: OUT types to the parent, SUM types eps'd), which reproduces (11.14)-(11.17'); chain matmuls (11.18'');
    Delta (second read); inter-node vectors in SMEM / VTCM.
15. Export v+1 (C15): base codes 2 = i, 3 = q; header q_i, q_q; spider marker 0xFE; program opcodes G1F / G2F (Sec. 14.1);
    flags.bit3 + the generator block of Sec. 13.7 (header, 32-B class records with type_sig / P_c / offsets / s_W / Cc,
    type classifier, E or zero-table flag, context gate, W_c / b_c blobs; param_offset 0xFFFFFFFF = generate) with the
    loader checks (exact Cc equality, T19 codec vectors, mix32(0)).
16. Circuit-form colouring with transparent spiders (11.22), Delta ancillas, Bell fallback for odd cycles; the uncompacted
    training circuit (11.23') for hardware-form tests and <= 2 state words only.
21. Tree adjoint (Sec. 11.7): root weights (11.27'), word cotangent (11.28), child cotangent (11.29) as the node op with a
    conj flag and one field role permuted, spider / chain adjoints, per-word Sec. 5.3 recurrence (11.30) with sign_k = +1,
    scatter-add over occurrences, reverse post-order schedule, M_train accounting (11.31).
17. MPO verb: two-stage contraction (12.14), isometry generation via U_A^T / U_C (12.15), operator-norm projection
    (12.16) by power iteration; qudit odometer (12.16').
18. Angle generator: (13.1)-(13.2) with the safe cast and the integer path; FNV-1a hashing; mix32 zero-table; integer
    mean; type classifier with FAIL-retry; segmented-reduction gradient scatter; BPTT for (13.9).
19. DisCoCirc: coref_resolve interface, build_gates with C17 argument resolution and emission order, permutation pi for
    i_subj > i_obj, compile-time switch over field pairs, readout (14.19), branch mixing (14.18).
20. MPS: two-site update (14.8)-(14.12) with rank guard, sorted one-sided Jacobi (14.13)-(14.14) incl. the m < n
    transposed case, centre moves by QR, renormalisation or log Z carry, swap network / wire reordering, single- and
    two-wire reduced density matrices, Uhlmann fidelity via two Hermitian Jacobi calls.

Tests (fp64 reference values computed by addendum/t18.c and t18b.c; tolerances fp32 1e-6, fp16-store 3e-3, Q1.15 2e-3
unless stated). Word states are those of Sec. 8 / T5: Alice = (0.3, 0.7, 1.1), Bob = (0.5, -0.4, 0.9) under Rx Rz Rx;
loves = IQP ansatz 0, L = 1, (t0, t1) = (0.8, -0.6); sleeps = n^r s, IQP ansatz 0, L = 1, t0 = 0.8, IV = [0.5,
0.46053050 - 0.19470917 i, 0.5, 0.46053050 + 0.19470917 i]; dreams = n^r s, IQP ansatz 0, t0 = -0.6, IV2 = [0.5,
0.47766824 + 0.14776010 i, 0.5, 0.47766824 - 0.14776010 i].

T11 Chart parser: every row of the Sec. 11.2 table returns exactly the listed cups and survivor with 1 parse; "Alice
    sleeps and dreams" returns (2,3) (1,4) (0,5) (8,9) (7,10), s at 6 (greedy must FAIL on it); "n n^l | n | n^r n n^l |
    n | n^r s" returns 2 parses and kth(0) != kth(1); accept_prefix on "Alice loves Bob the" (trailing n n^l) returns
    the prefix through "Bob"; random strings: chart == exhaustive search (0 false accepts / rejects, 1e5 strings).
T12 Relative clause at (1,1), "Alice who loves Bob sleeps", cups (0,1) (4,5) (7,8) (3,6) (2,9), survivor 10:
      t1[a + 2c] = sum_b V[a,c,b] Bob[b] = [0.27897688 - 0.15438512 i, 0.19683432 - 0.25083682 i, 0.18020560 - 0.07329415 i, 0.19452243 + 0.00266698 i]
      t2[a] = Alice[a] (t1[a] + t1[a+2])   = [0.25800180 - 0.30860466 i, -0.20244073 - 0.20369536 i]
      sigma = [-0.00389059 - 0.20869319 i, 0.07543213 - 0.28752732 i];  Z = 0.13192995;  p = [0.33023573, 0.66976427];  Loss(y=0) = 1.10794854
    via (a) the node-op post-order (11.14), (b) brute force over the 11-qubit product state with who = delta[a,b,d] eps[c]
    laid out as who[a + 2b + 4c + 8d] (agreement 7.9e-17 in fp64), (c) the circuit of Sec. 11.5 (final amplitudes =
    sigma x 2^{-1/2}, p_surv = Z/2 = 0.06596498). All angles zero: sigma = [0.35355339, 0.35355339], Z = 0.25.
    Object relative "Alice whom Bob loves sleeps", cups (0,1) (5,6) (4,7) (3,8) (2,9), whom[a + 2b + 4d + 8c]:
      sigma = [-0.00216678 - 0.20792288 i, 0.09629179 - 0.32243941 i];  Z = 0.15647590;  p = [0.27631489, 0.72368511]  (brute force 5.7e-17).
    Peak amplitude count of the node-op path must be 14 (10 with the slot rule); assert it equals N TV N's.
T13 Spider on q = 1 (11.7)/(11.9): mu(Alice, Bob) = [0.60005880 - 0.09685177 i, -0.38735435 + 0.06042272 i];
    eps(Alice) = 0.58494118 - 0.92099031 i; eps(Bob) = 0.78906577 - 0.43666705 i.
    "Alice and Bob sleep" (11.16): sigma = [0.13340576 + 0.05482207 i, 0.10987605 - 0.09602082 i]; Z = 0.04209530; p = [0.49417764, 0.50582236].
    "Alice sleeps and dreams" (11.17): sigma = [0.13129826 - 0.22615905 i, 0.16150588 - 0.23282446 i]; Z = 0.14867853; p = [0.45996654, 0.54003346].
    Hardware form of mu: CNOT(x -> y) + postselect y = 0 on |Alice>|Bob> gives mu(Alice, Bob) with amplitude factor exactly 1;
    eps by H + postselect |0> gives eps(Alice) x 2^{-1/2}.
T14 THAT orientation (11.14'): in "Alice believes that Bob sleeps" the contraction t_that[c] = sum_{c'} phi_that[c + 2 c']
    t_sleeps[c'] equals brute force to 1e-15; the transposed index order differs by O(1) (assert > 0.1).
T15 4x4 one-sided Jacobi SVD (14.13)-(14.14), fp64, tol 1e-15, on M = I + 2 P (P the cyclic shift: M[i][j] = (i == j) +
    2 (j == (i+1) mod 4)), a normal matrix with eigenvalues 1 + 2 i^k:
      S (sorted) = [3, 2.2360679775, 2.2360679775, 1] = [3, sqrt 5, sqrt 5, 1];  5 sweeps;  ||U S V^H - M||_max <= 2e-15;
      ||V^H V - I||_max <= 1e-15;  ||M||_F^2 = 20;  truncation to chi = 2 keeps 14, discards eps^2 = 6 (0.3 of the norm).
    Second case, the Jordan block J_4(2) = [[2,1,0,0],[0,2,1,0],[0,0,2,1],[0,0,0,2]]: S = [2.8512117716, 2.4329222531,
    1.8371875468, 1.2554770653], prod = 16 = det, sum of squares = 19 = ||M||_F^2; 5 sweeps. Without the sort step the
    returned order differs from descending in most random trials (assert the sort ran).
    Rank guard: Theta' from two |0> nouns and an identity gate has S = [1, 0, 0, 0]; the update must set chi_i = 1 and
    produce no NaN (verH/mps_nan.c is the negative test).
T16 Mixed-radix indexing for d = 3 (12.16'): verb fields (a, s, b) with radices (3, 3, 3), strides (1, 3, 9): (2,1,0) ->
    5; (1,2,2) -> 25; (0,0,2) -> 18; i = 17 -> (a, s, b) = (2, 2, 1). Radices (3, 2, 3), strides (1, 3, 6): i = 13 ->
    (1, 0, 2); (2,1,2) -> 17; the odometer visits 0, 1, ..., 17 in address order and terminates after 18 bodies.
T17 MPO verb (12.13)-(12.14), integer case d = 2, r = 2, d_s = 2: A[a r + r1] = [1, 2, 3, -1], C[r2 d + b] = [2, 0, 1, 1],
    B[r1 + r s + r d_s r2] = (r1+1)(s+2) - 3 r2 = [2, 4, 3, 6, -1, 1, 0, 3]; dense V[a + 2 s + 4 b] = [21, 0, 36, 3, 1, -4, 6,
    -3]; nouns n1 = (1, 2), n2 = (3, -1): u = (7, 0), w = (6, 2), sigma = (70, 126) by both the dense path and the two-
    stage streamed path; assert B is read once in address order.
T18 Circuit colouring (11.22): "Alice who loves Bob sleeps" -> states {loves, sleeps}, effects {Alice, Bob}, who
    transparent, W_hw = 3 at (1,1); "Alice and Bob sleep" -> W_hw = 3 (one Delta ancilla), amplitude equals (11.16) up to
    the postselection factors; "Alice who sleeps quickly runs" (triangle) -> exactly one cup marked Bell.
T19 Angle codec (13.2): z = -1.0 -> k = 60321; z = 0.8 -> 4172; z = 13.0 -> 2261; a NaN input -> k = 0; the integer path
    with s_E s_W chosen so that acc = 1000 corresponds to z = 0.8 returns 4172 +- 1. mix32(0) = 0x92CA2F0E, mix32(1) =
    0x96A0F96B, mix32(0x12345678) = 0x047660EA; FNV-1a("<the>") = 0xFCF0F1C0. |G_w| for ell = 1, 2, 3, 7 = 1, 3, 6, 22.
T20 DisCoCirc argument order (C17): "Alice loves Bob" with wires (Alice, Bob) = (0, 1) and with (1, 0) (text "Bob sleeps.
    Alice loves Bob.") gives identical rho_Alice to 1e-12; "Alice loves big Bob" applies U_big before U_loves and the
    object resolves to Bob through the adjective's n^l cup; a real Sim15 verb gate on complex nouns must FAIL a
    controlled-phase reproduction test (negative test for the SO(4) statement of 14.1), Sim14 must pass it.
T21 MPS vs exact (mpstest.c): chi = 8, N = 6, 30 random U(4) gates: ||Psi_mps - Psi_exact||^2 <= 1e-10; ./mpstest 2 11:
    error == discarded weight to 6 digits (1.379e-2) and ||Psi_trunc||^2 = 0.986212; renormalised variant has norm 1.
T22 Memory accounting: the node-op scheduler's reported peak equals (11.18') on the two-relative-clause sentence (16 /
    48 amplitudes, 12 / 40 in place) and on the Sec. 11.4 believes sentence (14 / 44); the streaming form reports 20 / 80.
T23 Precision (Sec. 12.3): over 400 Haar-random triples at (1,1), (2,1), (4,2) the mean Z is 0.25, 0.0625, 0.0039 +- 10 %;
    the Q1.15 (4,2) forward pass differs from fp32 by <= 2.5e-2 in p and the p_min flag fires when Z < 1.6e-3.
T24 Tree adjoint (Sec. 11.7; fp64 reference critic/adjtree.c, loss -ln p_0, words as above):
    (a) "Alice loves Bob" (root loves; children Alice, Bob): the 8 gradients equal Sec. 8's dLoss(y=0)/dtheta to 1e-7 in
        fp32 (fp64: 4.8e-11 against central differences h = 1e-5).
    (b) "Alice who loves Bob sleeps" (T12 tree: root sleeps; child who (spider: mu with Alice, eps over loves' s field);
        loves with child Bob): dL/dtheta = [-0.15106071, -0.14060757, 0.17728899 | 0.04493042, -0.01820026, 0.02841004 |
        0.03290603, -0.10412078 | 0.61601074] (Alice | Bob | loves | sleeps), fp64 central differences to 1e-6 (fp32 run),
        3.1e-11 (fp64); the scheduler reports M_train = 50 amplitudes at (1,1) by (11.31).
    (c) "Alice sleeps and dreams" (and_vp spider, OUT = {S}, SUM = {N}): [-0.03859959, -0.02433568, 0.02115842, 0.43442175,
        0.43442175]; the two IV cotangents come out of the spider adjoint as conj(Alice[a] IV2[a,s]) lambda_sigma[s] and
        conj(Alice[a] IV1[a,s]) lambda_sigma[s].
    (d) "[Alice loves Bob] and_s [Bob loves Alice]" with SHARED angle blocks (Alice, Bob, loves each occur twice; and_s =
        (11.13'), mu adjoint on the two sub-sentence vectors): [-0.02308977, 0.00129679, -0.02036071, -0.08223878, 0.01134526,
        -0.06765975, 1.06704471, 1.84830706]; sigma = [-0.07322709 + 0.01232450 i, -0.05891819 - 0.04440411 i], L = 0.68668626.
        Omitting the scatter-ADD of the second occurrence must FAIL this test (negative test).
    (e) The circuit adjoint of Sec. 5.3 on the uncompacted circuit of (a) (W = 3) and the tree adjoint agree to 1e-7; on
        (b) the circuit form needs W_train = 5 and must report 24 x 32 = 768 B against the tree's 400 B.

---------------------------------------------------------------------------------------------------------------

## 19. UNVERIFIED items

 1. Literature attributions and figures not checkable offline: Buszkowski 2001 (pregroup = CFG), Lambek 2008 question
    types, Sadrzadeh-Clark-Coecke relative-pronoun construction, Wijnholds-Sadrzadeh 2019 ellipsis, Shieber 1985,
    Coecke 2020 DisCoCirc and Duneau et al. 2024 gate set / actor counts, Perez-Salinas et al. 2020 and Schuld-Sweke-
    Meyer 2021, Grefenstette & Sadrzadeh / Kartsaklis dimensions, transformer sizes, Meichanetzidis et al. 2020 sizes,
    2507.02940 details, Miller et al. bond dimensions, TTLM margins, COGS / SCAN figures and the Csordas / Jiang-Bansal
    results, bert-tiny 4.4 M / ~83 % SST-2, MiniLM-L6 ~22 M and BERT-base ~110 M parameters, Bankova / Lewis hyponymy.
 2. The tokens-per-word ratio m ~ 2.3 x words; the 25-27 % cycle frequency and 46-58 % greedy failure rate hold for the
    CFG generator of refE/genE.c, not for natural text; likewise the W_train ~ 25-30 figure for a 20-word sentence at
    (2,1) in (11.23'') (the (n_unitary / 2) x mean Q_w law is exact for the 2-colouring, the word mix is assumed), and the
    mean 2^{q_out} = 4 per word used in the 20-word M_train figure of (11.31).
 3. Sufficiency of the dimension count (12.3) at L_sat for Sim14 / Sim15 beyond the Q <= 3 rank checks; which L
    saturates SO(d^2) for Sim15 and SU(d^2) for Sim14 as DisCoCirc gates; whether Sim14 / Sim15 reach the eigenvectors
    of an arbitrary density matrix (13.2(c)).
 4. Hardware: HVX (4,1) / (3,3) amplitude-per-lane register residency (marginal by count); vrmpy lane arithmetic beyond
    hvx_protos.h; HVX op counts for the zero-table mixer, the row sum and the vrmpy reduction; fp16-stored Jacobi
    accuracy; Adreno registers per fibre; CUDA register counts (ptxas targets); HMX tile rates; VTCM sizes on
    v66-v69 parts (8 MB on v73+ is confirmed by Sec. 3.2); Hexagon integer-divide cost.
 5. That string equality of tokenised mentions suffices for coreference on any data set; the accuracy of a rule-based
    (Hobbs-style) resolver; the accuracy of an averaged-perceptron tagger at 0.5-4 MB.
 6. Accuracy impact of B = 2^15 hash buckets (>= 10-way n-gram sharing) versus fastText's 2e6; usefulness of the raw-
    byte floor; the heuristic that real language tasks need q_n = 4..6.
 7. That training drives Z above the Haar-random 2^{-2 q_n} (which would re-enable fp16 / Q1.15 storage at q_n >= 5);
    the E[Z] = 1 statement of Sec. 1.2 is corrected here to 2^{-2 q_n} for Haar-random words.
 8. All expected outcomes of the evaluation plan (15.4 item 5); whether the compositional head beats a linear probe on
    the same encoder (15.3(d)); retrieval recall saturation at 64-256 complex dimensions (15.3(e)); the noun-inventory
    threshold for q_n = 2 (15.3(b)).
 9. The consistent aux-inverted question types of Sec. 11.2 against Lambek's; whether lambeq's IQP omits the closing H
    (C10 ansatz 0); nested vs component-wise cups in lambeq (C8).
