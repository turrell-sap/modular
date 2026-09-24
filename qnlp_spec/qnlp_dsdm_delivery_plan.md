# QNLP delivery plan (DSDM Agile Project Framework)

Version 1.0, 2026-09-24. Plain ASCII. No Python anywhere: product code is C / C++ / Mojo / CUDA; tooling is
shell, awk, jq, sha256sum, CMake or Bazel, nvcc / ptxas, the Hexagon SDK tools and the engineer's adapter.

Registry key: dsdm (bridge Sec. 12). Authority: this plan is authoritative for SCHEDULE, PRIORITY (MoSCoW) and
ACCEPTANCE SEQUENCING only. It makes no technical statement of its own: every number, convention, bound and
test value below is quoted from a registered document with a citation, and precedence rules R1-R13 of the bridge
(qnlp_bridge_for_agents.md Sec. 2.1) govern every one of them. Where this plan and a technical document seem to
disagree, the technical document wins and this plan is wrong.

Citation style (bridge Sec. 0): <key> Sec. <n> (<equation>) l<line>, keys spec, errata, addendum, companion,
review, limits, verify_c, puremath, bridge. sNNN / aNNN / cNNN are spec / addendum / companion lines. Test ids
T1-T24, T-EZ and the errata checks are those of the bridge test registry (bridge Sec. 7); limits tests are cited
as "limits <id>". Ideal day = one person-day of focused work with no interruptions (Sec. 10).

How to use this plan:
  - The engineer: read Secs. 1-3 once; run each timebox from its Sec. 5 entry; renegotiate MoSCoW at every
    timebox close-out by Sec. 11.3; take go/no-go decisions at the gates of Sec. 5.4.
  - A software agent: take a requirement by its PRL id (Sec. 4), follow the bridge task procedure named for its
    module (bridge Sec. 8; module map in Sec. 6), meet its acceptance criterion and the definition of done
    (Sec. 7.5), and report in the stand-up log format of Sec. 11.2. Agents never change MoSCoW, timebox dates,
    or any document of record.

Contents
  1. DSDM framing: principles, project approach questionnaire, preconditions
  2. Roles and responsibilities
  3. Lifecycle and DSDM products, phase by phase
  4. Prioritised Requirements List (PRL)
  5. Increments, timeboxes and go/no-go gates
  6. Solution Architecture Definition (summary)
  7. Development Approach Definition
  8. Risk log
  9. Traceability matrix
  10. Estimation
  11. Management approach
  12. Benefits
  Appendix A. Plan-level gap raised with the bridge (G31)


## 1. DSDM framing

### 1.1 Programme in one paragraph

The product is a QNLP evaluator and trainer: pregroup / DisCoCat grammar compiled to fused tensor contractions
or bent-wire circuits (spec Secs. 1-2), trained in fp32 on CUDA with adjoint gradients (spec 5.3, 6.1) and
exported (spec 6.3) for forward-only inference on Qualcomm Hexagon HVX (Q1.15) and Adreno through the adapter the
engineer has already written (spec 6.2, 3.2, 3.4), then extended to wider grammar (addendum Sec. 11), capacity and
hybrid front ends (addendum Secs. 12-13) and discourse (addendum Sec. 14, limits A1-A4). The business need is the
design goal "NLP on quantum theory with almost zero memory", which the errata sharpened into a claim about
materialised objects (0 B) and the per-sentence working set (40-400 B), NOT about the persistent angle table
(errata E-4). The first deliverable increment that matters is the minimal falsifiable programme P0 (review Sec. 7
l547-576): if P0 is falsified, the programme stops or is redirected at Week 20 (Sec. 5.4, gate G4).

### 1.2 The eight DSDM principles, applied

| # | DSDM principle | What it means in this project (concrete practice) | Where enforced |
|---|---|---|---|
| 1 | Focus on the business need | The need is the falsifiable P0 statement (review l561-563), not "a QNLP library". Every PRL row traces to a document section; the companion's enterprise / ACDOCA / Invoice-to-Payment / LNN profiles are Won't this time (review l573-576; bridge Sec. 11 "Deferred items"). Nothing in I5-I7 starts unless gate G4 passes. | PRL (Sec. 4), gate G4 (Sec. 5.4) |
| 2 | Deliver on time | Fixed 2-week timeboxes; dates never move, scope does: at close-out unfinished Coulds, then Shoulds, are dropped or re-planned; a Must that cannot finish triggers escalation (Sec. 11.4), never a timebox extension. The P0 decision date (end of Week 18, record by Week 20) is fixed from Foundations. | Timebox plans (Sec. 5), escalation (Sec. 11.4) |
| 3 | Collaborate | With a solo engineer, collaboration is with (a) software agents working to the bridge procedures, (b) the vacant business roles covered by interim arrangements and by the predeclared acceptance criteria that the documents fix in advance (review l561-563), and (c) the document authors via the errata process (bridge 8.8). Paraphrase authoring (P0-14) is deliberately given to someone other than the engineer. | Roles (Sec. 2), workshops (Sec. 11.1) |
| 4 | Never compromise quality | Quality level is fixed before development: every kernel is checked against the fp64 reference at the spec tolerances (fp32 1e-6, fp16-store 3e-3, Q1.15 2e-3; spec l1258), error bounds are those of errata E-11, residency claims are compiler-verified targets (addendum C14), and the baseline spec stays byte-identical (errata l3-8). Tests exist before the kernel they check (reference-first, Sec. 7.1). | Definition of done (Sec. 7.5), CI (FND-04) |
| 5 | Build incrementally from firm foundations | Foundations largely exist already (spec, errata, addendum, review, limits, puremath, bridge). I1 builds the fp64 reference and harness before any device kernel; I2 (CUDA) and I3 (Qualcomm) are checked against I1; I5-I7 build on the P0 result. Each increment is deployable and demonstrable (Sec. 5.1). | Increment plan (Sec. 5.1) |
| 6 | Develop iteratively | Each timebox runs Investigation / Refinement / Consolidation; kernels are refined against the reference until the acceptance value is met; the P0 early read at Week 8 (gate G1) feeds back into training choices before the device work. | Timebox structure (Sec. 5.2) |
| 7 | Communicate continuously and clearly | Daily written stand-up log (engineer + agent queue), timebox review records, a one-page status report every timebox (Sec. 11.5), demonstrations at every close-out, and the bridge / manifest as the single machine-readable registry of what is true. | Sec. 11 |
| 8 | Demonstrate control | Hash-pinned documents in CI, the PRL with effort and MoSCoW percentages computed (Sec. 4.3), gates with predeclared numbers, a risk log with closing tests (Sec. 8), and the traceability matrix (Sec. 9). | Secs. 4, 5.4, 8, 9 |

### 1.3 Project Approach Questionnaire (PAQ)

Answers: SA strongly agree, A agree, N neutral, D disagree, SD strongly disagree. Honest answers for a solo
research-engineering project; every D / SD has a mitigation and a risk id (Sec. 8).

| # | PAQ statement | Answer | Evidence / mitigation |
|---|---|---|---|
| 1 | The Business Vision is clearly defined and understood | A | "Almost zero memory" is precisely defined by errata E-4 (materialised objects 0 B; working set 40-400 B) and P0 is fully predeclared (review l549-565). Mitigation for the missing Visionary: the vision statement is quoted verbatim in the ToR (FND-01). |
| 2 | All parties understand and accept the DSDM philosophy | N | Only one person; agents follow the procedures they are given. Mitigation: Sec. 0 "How to use"; a DSDM Coach session is recommended (Sec. 2). |
| 3 | The Business Sponsor demonstrates clear and active ownership | SD | Role is VACANT (Sec. 2). Mitigation: the engineer drafts, a named sponsor signs the ToR and the G4 decision; until then decisions are recorded as "interim" (risk PR-02). |
| 4 | The Business Visionary is actively involved | D | VACANT. The review's P0 statement and addendum 15.5 stakeholder statements stand in. Risk PR-02. |
| 5 | Business Ambassadors and Advisors are available and have the time | SD | The weakest DSDM precondition here. Mitigation: acceptance criteria are predeclared numbers from the documents, so day-to-day acceptance does not need a person; the two items that do need an independent human (200 paraphrases, P0-14; G4 sign-off) are scheduled and budgeted (Sec. 2 recommendations). Risk PR-02, PR-12. |
| 6 | The Solution Development Team is empowered to make decisions within its timeboxes | SA | The engineer owns technical decisions within R1-R13 of the bridge. |
| 7 | The team has the appropriate skills | A | Engineer: CUDA, Hexagon, adapter author. Gaps: independent testing, evaluation statistics, linguistics. Agents cover volume, not independence (risk PR-05). |
| 8 | The team is co-located or has effective communication | SA | One person plus agents in the same repository; written logs. |
| 9 | On-time delivery of an acceptable solution is the primary measure of success | A | Accepted for timeboxes; the programme-level measure is the P0 verdict, including a negative one (companion 16.3). |
| 10 | Requirements can be prioritised (MoSCoW) and not everything is a Must | SA | Sec. 4: Musts 55 % of effort overall and <= 60 % in every timebox. |
| 11 | The solution can be delivered and demonstrated incrementally | SA | Eight increments, each demonstrable (Sec. 5.1). |
| 12 | High-level requirements can be baselined early | SA | They already are: spec Sec. 9, addendum Sec. 18, review Sec. 7. |
| 13 | Detailed requirements can emerge during the project | A | Gaps G1-G30 (bridge Sec. 11) are expected to be resolved as work reaches them; each resolution goes through the errata or registry procedure. |
| 14 | Testing is integrated throughout the lifecycle | SA | Reference-first harness in I1; every later kernel is checked against it (Sec. 7.1). |
| 15 | Appropriate tooling and environments are available | N | CUDA: likely. Hexagon: depends on a v65+ part and the adapter exposing VTCM reservation (spec 3.2 l637-641; review l564-565). Risk PR-03, PR-06. |
| 16 | Supplier / third-party dependencies are manageable | N | Toolchains (CUDA, Hexagon SDK) and datasets for later benchmarks (MC, RP, bAbI) are external; literature figures are UNVERIFIED (bridge Sec. 10 J). |
| 17 | Governance allows decisions to be made quickly | D | No sponsor. Mitigation: gate decisions predeclared; interim decisions recorded and ratified later (Sec. 11.4). |

### 1.4 Where DSDM preconditions are weak, and what is done instead

| Weak precondition | Consequence | Mitigation built into this plan |
|---|---|---|
| No business people (Sponsor, Visionary, Ambassador, Advisor) | Nobody outside the engineer accepts or rejects increments; risk of self-certification | Acceptance criteria are numbers fixed by the documents BEFORE the work (spec tests, errata tests, addendum T11-T24, review P0 thresholds); gates cannot be passed by argument, only by the listed test output. G4 needs an external signature (Sec. 5.4). |
| Solo developer | Single point of failure; no peer review; estimation has no history | Agents as Solution Developer / Tester capacity under written rules (Sec. 2.3); second-independent-pass convention for every new number (TST-01; errata l5-6); velocity calibrated in TB0 (FND-08). |
| Research uncertainty (the hypothesis may be false) | "Business value" may be a negative result | DSDM principle 1 is applied to the P0 decision itself: a falsified P0 is a delivered outcome (negative-result record, DEP-03; companion 16.3). |
| Hardware access and toolchain facts UNVERIFIED | Device timeboxes can stall | Device work starts only after the reference exists; every device fact is a closing test in the risk log (bridge Sec. 10 B-E). |
| Agent usage limits | Capacity can vanish mid-timebox | Planning capacity counts agents at 4 of 10 ideal days; Coulds/Shoulds absorb a loss of agent capacity (Sec. 10). |


## 2. Roles and responsibilities

### 2.1 Team-size assumption (explicit)

One human engineer at 1.0 FTE, who holds the Technical Coordinator and Solution Developer roles and, as an
INTERIM, the Team Leader and Project Manager duties. Software agents supply Solution Developer and Solution Tester
CAPACITY (not roles with authority): they implement and test PRL items under the bridge procedures, and their
output is accepted only by the engineer against the PRL acceptance criterion. No other named people exist; every
other role is a VACANCY. No person is invented.

### 2.2 Role map

| DSDM role | Level | Filled by | Status | Responsibilities in this project | Risk if unfilled | Recommendation |
|---|---|---|---|---|---|---|
| Business Sponsor | Project | -- | VACANCY | Owns the Business Case and budget (hardware, agent usage); signs the ToR and the G4 decision | Decisions are self-certified; nobody can stop a programme that should stop (risk PR-02) | Name the budget holder of the engineer's organisation as Sponsor for 1 hour per gate (G0, G1, G4, final). Until then record decisions as "interim, engineer" and ratify later. |
| Business Visionary | Project | -- | VACANCY | Owns the vision ("almost zero memory", errata E-4) and the benefits (Sec. 12); interprets them at gates | Scope drift into deferred companion profiles | A person who will use the result (e.g. the owner of an on-device NLP use case) for 1 hour per increment review. Interim: the review's P0 statement (l561-563) and addendum 15.5 statements are the vision text. |
| Technical Coordinator | Project | Engineer | FILLED | Owns the Solution Architecture Definition and the Development Approach Definition; technical authority within bridge R1-R13; decides errata proposals (bridge 8.8) | -- | -- |
| Project Manager | Project | Engineer (interim) | VACANCY (interim cover) | Delivery Plan, Management Approach, status reporting, escalation to the Sponsor | Planning and development compete for the same person; reports are self-assessed | Keep the PM duties to the one-page status and the gate records (Sec. 11.5); consider a part-time PM when the team grows (Sec. 2.4). |
| Business Analyst | Project / team | Engineer + agents (interim) | VACANCY (interim cover) | Maintains PRL, traceability matrix and acceptance criteria; keeps them tied to document sections | Requirements drift from documents | Agents regenerate Secs. 4 and 9 from the PRL source file at each close-out; the engineer checks citations. |
| Team Leader | Team | Engineer (interim) | VACANCY (interim cover) | Runs timeboxes: kick-off, stand-ups, close-out, Timebox Review Records | Ceremony skipped under pressure | Stand-up and review templates (Sec. 11) keep ceremony to < 10 % of capacity. |
| Business Ambassador | Team | -- | VACANCY | Day-to-day business acceptance; authors business scenarios (here: the 200 paraphrases, P0-14) | Paraphrase set biased toward the grammar by its own author (risk PR-12); acceptance self-certified | Recruit one person (linguist or future user) for ~2 days in TB8 and ~0.5 day per later increment review. Mitigation now: predeclared numeric criteria. |
| Solution Developer | Team | Engineer + agents | FILLED (agents partial) | Builds the Evolving Solution per PRL item | -- | Agent rules in Sec. 2.3. |
| Solution Tester | Team | Agents (partial) + engineer | FILLED (partial, not independent) | Writes and runs tests from the documents; independent of the developer of the same item | Same agent writes code and test from the same misreading (risk PR-05) | Rule: the test for a PRL item is written by a DIFFERENT agent session that reads only the documents and the bridge, never the implementation; second-independent-pass for new numbers (TST-01). |
| Business Advisor | Team (as needed) | -- | VACANCY | Subject-matter advice: NLP evaluation design, linguistics, statistics of seeds and splits | Evaluation flaws (leakage, weak baselines) undermine G4 | One evaluation-literate advisor for a 2-hour review of the split manifests (P0-03) in TB3 and of the P0 metrics table in TB8. |
| Technical Advisor | Team (as needed) | -- | VACANCY | Hexagon / Adreno / CUDA operations expertise; toolchain and device access | HVX facts stay UNVERIFIED (bridge Sec. 10 B); adapter limits unknown | A Hexagon performance engineer for TB6 kick-off (1-2 hours) and a CUDA reviewer for ptxas evidence in TB4. |
| Workshop Facilitator | Supporting | -- | VACANCY | Neutral facilitation of Foundations, gate and retrospective workshops | Engineer facilitates own decisions | Self-facilitate with the checklists of Sec. 11.1; bring in an external facilitator for the Foundations workshop and G4. |
| DSDM Coach | Supporting | -- | VACANCY | Advice on tailoring DSDM to a solo, agent-assisted project | Framework applied ritually or dropped | One coaching session in TB0 and one retrospective review after I4. |

### 2.3 Rules for agent capacity (Solution Developer / Solution Tester)

  1. An agent works on ONE PRL id at a time and follows the bridge procedure for its module (bridge Sec. 8;
     module to procedure map in Sec. 6.1).
  2. An agent may not edit spec, errata, review, limits, verify.c or the companion; it may draft an erratum or an
     addendum change for the engineer (bridge 8.8). The baseline stays byte-identical (errata l3-8).
  3. Test and implementation of the same PRL id come from different agent sessions (Solution Tester rule).
  4. Every number an agent reports carries its source (document line or program path); anything else is
     reported as UNVERIFIED (bridge R7).
  5. No Python is written, run or proposed (bridge header). Tooling is C/C++/Mojo, shell, awk, jq, CMake/Bazel,
     nvcc/ptxas, Hexagon SDK tools.
  6. Agent usage is budgeted per timebox (Sec. 10.2); when the budget is exhausted, the engineer drops Coulds first.

### 2.4 Scaling note: a 3-5 person team

| Team size | Role allocation | Effect on the plan |
|---|---|---|
| 3 | Engineer = Technical Coordinator + Team Leader; Developer 2 = CUDA (I2) then grammar (I5); Developer 3 = Solution Tester + Qualcomm path (I3) | I2 and I3 run in parallel after I1 (both depend only on I1 and the export format, EXP-01); P0 gate moves from Week 18 to about Week 14; total nominal duration about 28 weeks (planning estimate, UNVERIFIED). |
| 4 | Add a Business Analyst / Ambassador at 0.5 FTE (PRL, paraphrases, evaluation design) and a part-time Project Manager | Business roles become real; G4 no longer self-certified; I6 and I7 can overlap. |
| 5 | Add a dedicated Solution Tester; the Technical Advisor role filled from the Hexagon side | Independence of testing no longer relies on agent-session separation; about 22-24 weeks nominal (UNVERIFIED). |

For any team size the fixed points do not move: reference-first (I1 before device work), P0 before I5-I7, the
MoSCoW 60 % rule per timebox, and the document precedence rules.


## 3. Lifecycle and DSDM products, phase by phase

### 3.0 Overview

| Phase | When (week offsets) | Timeboxes | Principal output | Gate at exit |
|---|---|---|---|---|
| Pre-Project | Week 1, day 1 | -- | Terms of Reference | ToR accepted (interim) |
| Feasibility | Week 1 | TB0 (first half) | Feasibility Assessment, Outline Plan, Business Case outline | Feasible to proceed |
| Foundations | Week 2 | TB0 (second half) | Business Foundations, SAD, DAD, MAD, Delivery Plan, PRL | G0 |
| Evolutionary Development | Weeks 3-36 | TB1-TB17 | Evolving Solution, Timebox Plans and Review Records | G1-G7 per increment |
| Deployment (per increment) | Close-out of TB3, TB5, TB7, TB9, TB12, TB14, TB17; final Weeks 37-38 | -- | Deployed increment, Project Review Report (increment), Benefits Enablement | Increment accepted |
| Post-Project | Interim at Week 20 (after P0); final 12 weeks after the last deployment | -- | Benefits Assessment | -- |

Product status legend: EXISTS = provided by a registered document (section named); DRAFT = drafted in this plan;
MISSING = no document provides it yet (the gap id says why).

### 3.1 Pre-Project

Objectives: confirm that the programme is worth starting, name its scope and constraints, and set up the
Feasibility work.

Activities: (1) read the bridge Secs. 0-3 and the manifest; (2) confirm the seven pinned hashes (bridge Sec. 1);
(3) state scope as "baseline spec + errata + addendum, gated by P0" and state the deferred scope (companion Secs.
10-12.8 and 14; review l573-576); (4) record constraints: C / C++ / Mojo only, no Python, classical simulation on
CUDA and Qualcomm through the existing adapter; (5) identify the vacant roles (Sec. 2).

| Product | Owner | Status | Provided by | Still missing |
|---|---|---|---|---|
| Terms of Reference | Business Sponsor (VACANCY; drafted by the engineer) | DRAFT (this section plus Sec. 1.1) | review Sec. 1 l11-37 (governing status), Sec. 7 l547-576 (P0); bridge Sec. 0 and Sec. 11 "Deferred items"; spec Sec. 7 as corrected by errata E-4 (what "almost zero memory" means) | Sponsor signature; hardware and agent-usage budget |

Terms of Reference (draft): Objective: build and evaluate, in C / C++ / Mojo, a pregroup / DisCoCat evaluator
and trainer whose per-sentence working set is 40-400 B and whose materialised objects are 0 B (errata E-4),
trained on CUDA and inferred on Qualcomm through the adapter, and decide by the P0 falsifiers (review l562-563)
whether to continue into wider grammar, capacity and discourse. Scope in: spec Secs. 0-9 with errata E-1..E-13;
addendum Secs. 11-18; limits themes A1-A4 as Should/Could in I7. Scope out (Won't this time): companion Secs.
10-12.8 and 14, companion 13.4 profiles other than P0, MCU profiles (bridge G24), lambeq weight import, anything
in Python. Constraints: frozen baseline (errata l3-8); precedence R1-R13; no device facts claimed without a
measurement (bridge R7). Budget and sponsor: to be named.

Entry criteria: the document set exists and its hashes match bridge Sec. 1. Exit criteria: ToR accepted
(interim acceptance by the engineer is recorded as such); Feasibility scheduled.

### 3.2 Feasibility

Objectives: establish whether the solution is technically feasible and worth building, and sketch the plan.

Activities: (1) technical feasibility desk check: verify.c rebuilt (cc -O2 verify.c -lm) and its printout diffed
against spec Sec. 8 (bridge Sec. 0 step 4); (2) hardware feasibility check: CUDA GPU available; Hexagon part
generation (v65+ needed for vgather, errata E-9 l114, else sorted batch) and adapter capabilities listed (VTCM
reservation, HVX context, spec 3.2 l637-641); (3) business feasibility: can P0 be run with the available people
(paraphrase author, sign-off); (4) outline plan and business case outline.

| Product | Owner | Status | Provided by | Still missing |
|---|---|---|---|---|
| Feasibility Assessment | Technical Coordinator (engineer) with Business Visionary (VACANCY) | DRAFT (below) | Technical: spec Secs. 1-9 (model, kernels, formats, memory), errata E-1..E-13, addendum Secs. 11-18, review Sec. 2 (every correction re-derived with fp64 programs, review l578-603), verify.c; hardware: spec Sec. 3 and Sec. 10 items 2-6 (UNVERIFIED), limits Sec. 4 (primitive costs) | Any device measurement (bridge G24): no ptxas report, HVX bring-up, latency or energy figure exists anywhere |
| Outline Plan | Project Manager (interim: engineer) | DRAFT (Sec. 5.1) | review l564-565 gives the P0 budget (one engineer, two weeks CPU + one CUDA GPU for residency, HVX port in week three); companion 15 c857-875 gives stages R0-R9 | Calibrated velocity (FND-08) |
| Business Case (outline) | Business Visionary (VACANCY) | DRAFT (Sec. 12) | review Sec. 7 (predeclared outcomes and falsifiers); addendum 15.4 l1612-1649, 15.5 l1650-1672, Sec. 17 l1744-1789; errata E-4 (claim scope); companion 13.5 c735-746 (advancement gates) | Money costs; a named benefit owner; the deployment context that would use the result |

Feasibility Assessment (draft). Technical: feasible on paper and reproduced numerically on the host: every Sec. 8
number is printed by verify.c; every errata correction has a plain-C program (review Sec. 8 l593-603); the
spec and addendum tests T1-T24 have reference programs except T2, T9 (needs the target), T16, T17, T20, T22 (bridge Sec. 7, G10). Hardware: feasible
with UNVERIFIED facts: CUDA register residency is a ptxas target (addendum C14 l17-24); HVX Q1.15 (1,1)
sentence-per-lane fits 27-29 of 32 registers by count (errata E-9 l108-110), not by measurement; HMX use is
conditional on the adapter (spec 3.3 l731-734). Business: feasible only with the Sec. 2 recommendations; the P0
experiment is small (1,190,592 distinct sentences, review l550; 176 / 320 angles, review l551-552). Verdict:
proceed, with the P0 early read (G1) placed at Week 8 to fail fast.

Entry criteria: ToR accepted. Exit criteria: Feasibility Assessment, Outline Plan and Business Case outline
exist; hardware check done or its absence recorded as a risk (PR-03); decision "proceed to Foundations" recorded.

### 3.3 Foundations

Objectives: baseline the requirements, architecture, development and management approach, and the delivery plan,
to the depth needed to start timeboxing; no deeper (DSDM "enough design up front").

Activities: Foundations workshop (Sec. 11.1): MoSCoW the PRL; walk the module map; agree tolerances, gates and
the definition of done; set up the repository, CI jobs (no Python, ASCII, hash pins: FND-04), and the tracker.

| Product | Owner | Status | Provided by | Still missing |
|---|---|---|---|---|
| Business Foundations (vision, benefits, business case) | Business Visionary (VACANCY) | DRAFT (Secs. 1.1, 12) | errata E-4 (claim scope), review Sec. 7, addendum 15.5 (two stakeholder statements), Sec. 17 (answers to the two criticisms) | Named Visionary; a deployment context |
| Solution Architecture Definition | Technical Coordinator | DRAFT (Sec. 6) | spec Secs. 1-7 and Sec. 9 items 1-11 (l1239-1256); addendum C14-C17 (l15-70), Secs. 11-16, Sec. 18 items 12-21 (l1790-1813); errata E-5, E-6, E-9, E-11, E-13; limits Sec. 4 (primitive costs); bridge Sec. 3 (conventions), Sec. 9 (numbers) | Adapter interface (engineer's code; no document describes it); binary encoding of cups[] / tree[] (G4); output status record (G20); HVX layout for (1,3) (G31, Appendix A) |
| Development Approach Definition | Technical Coordinator | DRAFT (Sec. 7) | spec Sec. 9 tests and tolerances (l1257-1287); addendum Sec. 18 tests (l1814-1898); errata tests; bridge Sec. 7 (test registry), Sec. 8 (task procedures), 8.8 (errata process), Sec. 12 (registry update) | Assertion harness and one-command reproduction (G10; companion 16.1); programs for T2, T16, T17, T20, T22; the T9 tolerance (G9) |
| Management Approach Definition | Project Manager (interim) | DRAFT (Sec. 11) | DSDM practice; companion 13.5 (gates), 16.3 (negative results) | People for the vacant roles |
| Delivery Plan | Project Manager (interim) | DRAFT (Sec. 5) | review l564-565 (P0 budget); companion 15 (stages); this plan Sec. 10 (estimates) | Calibrated velocity after TB1-TB2 |
| Prioritised Requirements List | Business Analyst (interim: engineer + agents) | DRAFT (Sec. 4), baselined at G0 | spec Sec. 9; addendum Sec. 18; errata; review Sec. 7; limits Tables A/B (l35-124) | Business acceptance of the MoSCoW split (vacant business roles) |

Entry criteria: Feasibility exit met. Exit criteria (gate G0, end of Week 2): PRL baselined with Musts <= 60 %
per timebox (Sec. 4.3); SAD and DAD reviewed; CI jobs FND-04 green; TB1 Timebox Plan agreed; hardware access
either confirmed or its absence entered as risk PR-03 with the fallback (simulator or sorted batch) chosen.

### 3.4 Evolutionary Development

Objectives: build the Evolving Solution increment by increment in fixed timeboxes, against the fp64 reference.

Activities per timebox (Sec. 5.2): Kick-off, Investigation, Refinement, Consolidation, Close-out; daily stand-up
(Sec. 11.2); MoSCoW renegotiation at close-out (Sec. 11.3).

| Product | Owner | Status | Provided by | Still missing |
|---|---|---|---|---|
| Timebox Plans | Team Leader (interim: engineer) | DRAFT for TB0-TB17 (Sec. 5.5) | This plan | Refinement at each kick-off |
| Timebox Review Records | Team Leader (interim) | Template DRAFT (Sec. 5.3) | This plan | Filled at each close-out |
| Evolving Solution (code, tests, evidence) | Solution Development Team | MISSING (to be built) | Reference values: verify.c; addendum/t18.c, t18b.c, mix.c; critic/adjtree.c; derivH/, verH/, verE/, refE/, derivF/, second/, verEZ/, verQ15/, bp/, spsa_var.c, limits programs (bridge Sec. 1 folder table) | Everything else |

Entry criteria: G0 passed. Exit criteria: all increments delivered or the programme stopped at a gate.

### 3.5 Deployment

Deployment happens at the end of every increment (DSDM incremental delivery): the increment's code, tests,
evidence and documentation are tagged, hash-recorded and made usable by others.

| Product | Owner | Status | Provided by | Still missing |
|---|---|---|---|---|
| Deployment Plan (per increment) | Project Manager (interim) with Technical Coordinator | DRAFT (table below) | companion 16.1 c895-910 (evidence package contents) | Target users and hosting of releases |
| Project Review Report (per increment, and final) | Project Manager (interim) | Template DRAFT (below) | companion 16.3 c918-921 (negative results); DSDM | Filled per increment |
| Benefits Enablement | Business Visionary (VACANCY) | DRAFT (Sec. 12.3) | review Sec. 7 metrics; addendum 15.4 | Owner |

Deployment Plan per increment:

| Increment | Deployment week | What is deployed | Acceptance to deploy |
|---|---|---|---|
| I1 | end of Week 8 | fp64 reference library, harness, export v1, host trainer, P0 data generator; release r0.1 | T1-T8, T10, T-EZ, E-10 tests green; G1 recorded |
| I2 | end of Week 12 | CUDA forward + training for P0 shapes; ptxas evidence; release r0.2 | G2 criteria (Sec. 5.4) |
| I3 | end of Week 16 | HVX / Adreno inference via the adapter; device measurement notes; release r0.3 | G3 criteria |
| I4 | end of Week 20 | P0 result, evidence package, negative-result record, one-command reproduction; release r1.0 | G4 decision recorded (pass or fail) |
| I5 | end of Week 26 | Chart parser, build_tree, node ops, tree adjoint, export v+1; release r1.1 | G5 |
| I6 | end of Week 30 | MPO verbs, angle generator, generator export block; release r1.2 | G6 |
| I7 | end of Week 36 | DisCoCirc, MPS, spectral readouts; release r1.3 | G7 |
| Final | Weeks 37-38 | Consolidated release, final Project Review Report, Benefits Assessment plan | Sponsor acceptance |

Project Review Report template (per increment): increment id and dates; PRL items delivered by MoSCoW (Must /
Should / Could delivered vs planned, effort actual vs estimate); tests passed with values; gates passed and the
evidence; UNVERIFIED items closed (bridge Sec. 10 group letter, dated measurement note); errata or addendum
changes proposed (ids); negative results; benefits measured so far (Sec. 12); lessons for the next increment.

### 3.6 Post-Project

| Product | Owner | Status | Provided by | Still missing |
|---|---|---|---|---|
| Benefits Assessment | Business Visionary (VACANCY) | Plan DRAFT (Sec. 12.4) | review Sec. 7 predeclared outcomes; addendum 15.4 item 5 expected outcomes (hypotheses) | Owner; the deployment context in which benefits are realised |

The interim Benefits Assessment is taken at Week 20 on the P0 result; the final one 12 weeks after the last
deployment (Week 50 nominal), or 12 weeks after a stop decision at G4.


## 4. Prioritised Requirements List (PRL)

### 4.1 MoSCoW rules used

| Priority | Meaning in this plan | Test |
|---|---|---|
| Must (M) | Part of the Minimum Usable SubseT: without it the increment cannot be demonstrated, or the gate at the end of the increment cannot be evaluated. Musts come from P0 (review Sec. 7), the errata tests, the fp64 reference, the register-resident fused tensordot, CUDA training with adjoint gradients, and inference through the Qualcomm adapter; in I5-I7, from the addendum Sec. 18 checklist items and tests that define the increment. | "If this is missing, do we still ship the increment / take the gate?" -- No => Must |
| Should (S) | Important, painful to leave out, but a workaround exists; first to be re-planned into the next timebox when a Must overruns | Workaround exists and is stated |
| Could (C) | Desirable; the timebox contingency; dropped first | Dropping it changes no gate |
| Won't this time (W) | Agreed out of scope for this project; listed so that nobody builds it | Deferred by a document (review, bridge Sec. 11) or forbidden by one (spec, errata) |

DSDM effort rule applied: Musts <= 60 % of the estimated effort of every timebox and every increment; Shoulds
~20 % and Coulds ~20 % are the contingency that protects the Musts and the fixed end dates. The rule is checked
mechanically on the PRL source file (Sec. 4.3). Effort is in ideal days (basis: Sec. 10.1).

### 4.2 The list

Columns: ID; requirement; MoSCoW; source (document + section / line / equation); acceptance criterion (test id
and expected value where one exists); increment; timebox; dependencies; effort (ideal days). Test ids in the
acceptance column are defined in the bridge test registry (bridge Sec. 7) or, for plan-level checks (CI-*,
P0-*, PTXAS, HVX-REG, REPRO, ...), in Sec. 7.1 of this plan.

| ID | Requirement | MoSCoW | Source | Acceptance criterion | Inc | TB (weeks) | Deps | Effort (d) |
|---|---|---|---|---|---|---|---|---|
| FND-01 | Terms of Reference, Feasibility Assessment and Business Case outline agreed; P0 falsifiers quoted verbatim | M | review Sec. 7 l547-576; companion 13.5 c735-746, 15 c857-875 | Foundations sign-off record names the four P0 falsifiers of review l562-563 | I0 | TB0 (W1-2) | - | 1.0 |
| FND-02 | PRL baselined with MoSCoW; Musts <= 60 % of effort per timebox and per increment | M | DSDM MoSCoW rule; bridge Sec. 7 | awk effort check (Sec. 4.3 of this plan) shows Must share <= 60 % in every timebox | I0 | TB0 (W1-2) | FND-01 | 1.0 |
| FND-03 | Solution Architecture Definition: module map (spec Sec. 9 items 1-11, addendum Sec. 18 items 12-21) and the adapter boundary header in C | M | spec Sec. 9 l1239-1256; addendum Sec. 18 l1790-1813; spec 6.2 l1100-1110 | SAD reviewed at the Foundations workshop; adapter header compiles with cc -std=c99 and nvcc | I0 | TB0 (W1-2) | FND-01 | 1.5 |
| FND-04 | Development Approach Definition: CMake (or Bazel) build, ctest skeleton, CI jobs rejecting any Python file and any non-ASCII byte, and a hash-pin job checking the seven SHA-256 values of bridge Sec. 1 | M | bridge header, Sec. 0 hard rules, Sec. 1, R13; errata l1; companion 16.1 c895-910 | CI fails on a planted *.py file, a planted non-ASCII byte and a changed pinned document; sha256sum -c passes on all seven | I0 | TB0 (W1-2) | - | 2.0 |
| FND-06 | Environment readiness: CUDA toolchain + one GPU; Hexagon SDK + the engineer's adapter + a v65+ part (vgather) or the sorted-batch plan | S | review l564-565; errata E-9 l114; spec Sec. 10 items 2, 6 | Toolchain versions recorded; a trivial kernel runs on the GPU and through the adapter on the DSP | I0 | TB0 (W1-2) | - | 1.5 |
| FND-07 | Risk log seeded from bridge Sec. 10 (A-O), Sec. 11 (G1-G30) and puremath App. B; evidence labels adopted in status reports | S | bridge Secs. 10-11, G23; puremath App. B l3686-3745; companion 1.3 c66-77 | Risk log of this plan Sec. 8 in the tracker; status template carries an evidence label per claim | I0 | TB0 (W1-2) | - | 0.5 |
| FND-08 | Apply the addendum edits owed by the errata and review as an addendum revision with a new hash pin; time the first agent task against the engineer (velocity calibration) | C | bridge Sec. 10 item O, G22; errata E-4, E-9, E-11, E-12; this plan Sec. 10 | New addendum hash in bridge Sec. 1 and manifest; every aNNN citation re-checked; calibration hours recorded | I0 | TB0 (W1-2) | FND-04 | 2.0 |
| REF-01 | Index helpers i0, i1, i00, compact (C1) | M | spec 2.1-2.2 l356-413; spec T1 l1259 | T1: i0 bijective onto {bit_k = 0}, i00(j) >= j, compact(i00(j)) == j for all lo < hi at W = 6 | I1 | TB1 (W3-4) | FND-04 | 0.5 |
| REF-02 | Gates Rx, Ry, Rz, H, CNOT, CZ, CRz, CRx defined by bit tests; apply_1q / apply_2q; T2 program written (none exists on disk) | M | spec C5-C6 l32-60, C13 l115-142, (0.1)-(0.8); bridge G10 | T2: apply == dense M psi at W = 6, both control orientations, lift_lo / lift_hi, fp64 to 1e-12 | I1 | TB1 (W3-4) | REF-01 | 1.0 |
| REF-03 | Gate identities including the CRx check that verify.c does not code | M | spec T3 l1263; bridge Sec. 7 T3 | T3: CRz decomposition residual < 1e-15 (verify.c 1.68e-16); CRx = Rx CZ Rx CZ; CNOT sandwich != I asserted; U^T by reversed list == transposed dense for IQP and Sim15 | I1 | TB1 (W3-4) | REF-02 | 0.5 |
| REF-04 | Word-state generator for ansatz ids 0-4 with gate-list order C4 and field layout C7 | M | spec C4 l28, C7 l62-71, C10 l86-102 | T4: Alice [0.71847188-0.31582980i, -0.13353070-0.60516052i]; Bob [0.74959627+0.19470917i, 0.03946950-0.63137622i]; loves abs(V[k]) = 0.35355339 | I1 | TB1 (W3-4) | REF-02 | 1.0 |
| REF-05 | Fused tensordot over field lists with the E-13 slot rule (one 8 B accumulator) | M | spec 1.3 l225-267; errata E-13 l194-204 | T5: sigma = [0.17991068-0.41061012i, 0.41557129-0.46260942i], Z = 0.58767549; T6(a),(b): Z = 0.14026553, p = [0.58263882, 0.41736118] | I1 | TB1 (W3-4) | REF-04 | 1.0 |
| REF-06 | Bent-wire circuit form with the Sec. 1.4 wire-map table; postselect-compact in descending order; monolithic Bell-cup path | M | spec 1.4 l268-328, 2.3 l414-464 | T6(c): psi[0], psi[2] == sigma to 1e-16, D = Z; T6(d): P_post = Z/4 = 0.03506638; T7 six wire-map rows == tensordot to 1e-12 | I1 | TB1 (W3-4) | REF-05 | 1.0 |
| REF-07 | Readout N_c, D, P_c with Z_min = 1e-12, eps = 1e-9, eps_D = 1e-7, widen-before-square | M | spec 1.5 (1.6)-(1.7) l329-340; spec 5.1 l961-971 | T5: p = [0.34197193, 0.65802807]; T6: Loss(y=0) = 0.54018780, Loss(y=1) = 0.87380330; all-zero angles Z = 0.25 | I1 | TB1 (W3-4) | REF-05 | 0.5 |
| TST-01 | Assertion-based one-command harness (ctest) at the spec tolerances fp32 1e-6, fp16-store 3e-3, Q1.15 2e-3; every new expected value produced by two separate programs | M | bridge G10, 8.8 step 3; spec l1258; errata l5-6; companion 16.1 c897 | ctest runs T1-T6 and exits non-zero on any failure; each new value cites two agreeing programs | I1 | TB1 (W3-4) | FND-04 | 0.5 |
| REF-08 | Matmul form (2.8) and X-only flip-mask relabelling (2.4) | S | spec 2.4 l465-473, 2.5 l474-485 | T6(a) via the matmul form == (1.5) to 1e-12; CNOT never treated as a flip mask (spec l470) | I1 | TB1 (W3-4) | REF-05 | 1.0 |
| REF-09 | Fidelity between two sentence states (1.5) | S | spec 1.5 l335; limits A3 l591-611 | limits A3-gram F_01 = 0.97273229 reproduced in fp64 to 1e-8 | I1 | TB1 (W3-4) | REF-07 | 0.5 |
| TST-02 | verify.c regression job: cc -O2 verify.c -lm, printout diffed against spec Sec. 8 | S | bridge Sec. 0 step 4; bridge Sec. 1 verify_c row | Printout matches Z = 0.14026553 and every Sec. 8 value; verify.c hash checked | I1 | TB1 (W3-4) | FND-04 | 0.5 |
| REF-10 | Alternative reference paths: Q1.31 (int32 state, int64 accumulate) and the early-cup monolithic evaluator | C | spec 4.3 l799-829, 2.3 l414-464 | Per-gate error <= g_Q31(W) = 8.5e-8 at W = 16 (spec l826); T6(d) via early cups with peak 24 amplitudes (spec l459) | I1 | TB1 (W3-4) | REF-02, REF-06 | 2.0 |
| REF-12 | Circuit adjoint (5.6)-(5.8) on the UNFUSED gate list; sign_k = -1 for a transposed Ry; scatter-add | M | spec 5.3 l1009-1043; review l320-321; bridge R11 | T7: adjoint == central FD (h = 1e-6) to 1e-6 on the 8 parameters, dLoss(y=0)/dtheta = [-0.1204452, ..., 0.9490759]; Sim15 effect-word sign check | I1 | TB2 (W5-6) | REF-06 | 1.5 |
| REF-13 | Tensordot reverse rule for the six shapes (direct-factor reverse rule, tree form, sign_k = +1) | M | errata Sec. B l210-211; addendum 11.7 (11.27')-(11.30) l501-545 | T24(a): 8 gradients == Sec. 8 dLoss to 1e-7 (4.8e-11 in fp64) | I1 | TB2 (W5-6) | REF-05 | 1.0 |
| ERR-01 | fp16 accumulation ban with the E-7 / E-8 stall tests | M | errata E-7 l89-93, E-8 l95-98; spec C12 l110-113 | T8 (E-7 form): fp16 sum(2048) == sum(2049) == sum(4096) == 1.0; 2^-12 series stalls at 0.5; fp32 path to 1e-6 | I1 | TB2 (W5-6) | TST-01 | 0.5 |
| ERR-02 | Q1.15 tensordot emulation: pre-scale S_q15 = 1 - 2^-15, store sat_int16((acc + 2^14) >> 15); gate path >> 14 | M | errata E-6 l77-87; spec l810, l918 | T8: endpoint narrowing 1.53e-5 <= 2^-15 with pre-scale (3.41e-5 without); worked example abs(delta p0) 3.11e-5 / 3.47e-5 vs 2e-3; sweep max norm(delta sigma) 1.087e-4 <= 1.96e-4 | I1 | TB2 (W5-6) | REF-05 | 1.0 |
| EXP-01 | Export writer and loader, format v1 (32 B header, 20 B records, uint16 half-angle turns) | M | spec 6.3 l1111-1142; errata E-3, E-5; bridge Sec. 3.4 | T10: magic, stride 20, type_sig n^r s n^l == 08 01 04 FF FF FF, decode(4172) = 0.399985 +- 1e-6, 0.8 + 2 pi -> different k; E-5 offsets == param_offset; V = 1000 Sim14 file 32,032 B | I1 | TB2 (W5-6) | REF-04 | 1.0 |
| REF-15 | Host trainer: Adam (5.9) at 16 B/param, unwrapped fp32 theta, clip 1, uniform [0, 2 pi) init with fixed seed, >= 3 restarts | M | spec 5.5 l1054-1075; errata E-10 l126-147 | E-10 tests: d2P_0/dt0 dt6 at theta = 0 == -0.25; E[D] at uniform init == 0.2500 at abs(K) = 2; dN_0/dtheta == 0 for Bob t2 and loves t1 | I1 | TB2 (W5-6) | REF-12 | 1.0 |
| REF-14 | Gradient cross-checks: two-term + quotient and four-term shift rules, 4 pi periodicity, and the E-2 SPSA variance test | S | spec 5.2 l972-1008, 5.6 l1076-1087; errata E-2 l30-39 | T7: Alice t0 = 0.0701761; loves t0 = -0.2073400; two-term on CRz (-0.0684908) asserted wrong; p_0(t + 4 pi) = 0.58263882 != p_0(t + 2 pi) = 0.87639280; E-2: variance 0.466300 flat in c at P = 8 | I1 | TB2 (W5-6) | REF-12 | 1.0 |
| ERR-04 | E-11 readout policy: certificate 2e/(2 sqrt D - e), single D floor 2^-6 on both paths, fp32 fallback flag | S | errata E-11 l149-165, E-10 l145 | T8 P_c part: P_c error <= 2e/(2 sqrt D - e); fp16-store bound at D = 2^-6 is 0.0197 (passes 0.05); D < 2^-6 routed to fp32 | I1 | TB2 (W5-6) | REF-07 | 1.0 |
| ERR-06 | Host angle codec (octant reduction, degree-7/8 Taylor, optional 257-entry Q2.14 table of 514 B) and the E-9 65536-code sweep emulated on the host | C | spec 4.4 l830-843; errata E-9 l100-124; review l195 | sin <= 3.2e-7, cos <= 4e-8, degree 5/6 asserted to fail 2^-15; sweep 3.074e-5 vs 3.052e-5 + 3.1e-7 (tolerance open, G9) | I1 | TB2 (W5-6) | REF-04, ERR-02 | 1.5 |
| REF-17 | Real-only model path (re[] plane only, flags.bit2 = 1) for Sim15 / Ry-only words | C | spec C3 l22, C10 l86-102, Sec. 10 item 8 | Sim15 word states == complex path with zero imaginary part to 1e-12; memory halved | I1 | TB2 (W5-6) | REF-04 | 0.5 |
| ERR-03 | T-EZ Monte Carlo on the reference | M | errata E-1 l13-28 | T-EZ: mean Z over 1e5 Haar triples 0.250 +- 0.002 at (1,1), 0.0625 +- 0.0005 at (2,1) | I1 | TB3 (W7-8) | REF-05 | 0.5 |
| P0-01 | P0 CFG generator (S -> NP VP; NP -> N or ADJ N; VP -> TV NP or IV or IV ADV), 64-word lexicon (24 N, 12 ADJ, 12 TV, 8 IV, 8 ADV), and the label function (label(A TV B) != label(B TV A) on every role-swap pair; K = 2 for Model A, K = 8 for Model B) | M | review Sec. 7 l549-550, l554-555; bridge G25 | Distinct-sentence count == 1,190,592 (scope/exp0.c); all six shapes present; exhaustive role-swap check has 0 violations | I1 | TB3 (W7-8) | FND-04 | 1.5 |
| P0-03 | Split manifests and seeds: 4,000 train / 1,000 IID / 500 role-swap / 300 held-out triples / 200 paraphrase slots; 5 seeds | M | review l555-557; bridge G25 | Manifests hash-pinned; no held-out triple in train; each role-swap pair seen only in the other order | I1 | TB3 (W7-8) | P0-01 | 0.5 |
| P0-04 | Greedy shift-reduce parser and program compile for the six shapes (host), with parse-FAIL status | M | spec 1.1 l146-194, 2.6 l486-511, Sec. 9 item 1; errata E-12 | T5/T6 parse steps; greedy exact on every generated P0 sentence (review l550) | I1 | TB3 (W7-8) | REF-06 | 1.0 |
| P0-05 | Model A: (1,1), IQP ansatz 0, L = 2, host training, seed 1 | M | review l550-552 | 176 angles = 352 B; peak 16 amplitudes (10 in place) = 128 B fp32; accuracy per split reported | I1 | TB3 (W7-8) | REF-15, P0-03 | 1.0 |
| P0-06 | Direct-factor model at equal bytes (spec 1.6 vector model) as the ablation of the quantum parameterisation | M | review l558-559; spec 1.6 l341-353; companion c450 | Trained on the same split; accuracy per split beside Model A | I1 | TB3 (W7-8) | P0-03 | 1.0 |
| P0-07 | Gate G1 early-read decision record | M | review l562-563; this plan Sec. 5.4 | Record: seed-1 IID >= 90 % and role-swap >= 90 %, or a redirect decision | I1 | TB3 (W7-8) | P0-05, P0-06 | 0.5 |
| P0-08 | B1 bag-of-embeddings baseline (528 B, int8, own C++ autograd) | S | review l557; addendum 15.4 item 4 l1634-1642 | B1 role-swap == 50 % by construction (review l555) | I1 | TB3 (W7-8) | P0-03 | 0.5 |
| P0-09 | Model B: (1,3), host training, seed 1 | S | review l552-553; addendum 15.3(a) l1553-1558 | 320 angles = 640 B; 52 amplitudes = 416 / 208 B; K = 8 | I1 | TB3 (W7-8) | P0-05 | 1.0 |
| ERR-07 | E-12 regression, greedy half: greedy must FAIL (never mis-parse) on both counterexamples | S | errata E-12 l167-192 | Greedy returns FAIL on "Alice loves man in park" and "Alice loves the man in park" | I1 | TB3 (W7-8) | P0-04 | 0.5 |
| P0-10 | Init diagnostics and op counter: D histogram and clip rate at uniform init; counted ops per sentence | C | errata E-10 l137, E-1 l19; review l560 | Sec. 8 shape at uniform init: ~1.1 % of sentences with D < 2^-6, clip rate 52-54 %; op counter equals the hand count for the Sec. 8 sentence | I1 | TB3 (W7-8) | REF-15 | 1.0 |
| TST-04 | Small distributable fixture set with expected outputs | C | companion 16.1 c901-902 | Fixture file loads in the harness; expected outputs from fp64 | I1 | TB3 (W7-8) | TST-01 | 1.0 |
| CUD-01 | CUDA Layout A thread-per-sentence fused tensordot, fp32, (1,1) and (1,3), with device word states from uint16 k via sincospif(k/32768.0f); one instantiation per (shape, q_n, q_s, ansatz, L); compile-time indices | M | spec 3.1 l514-545, C5 l39-45, Sec. 9 items 2-3; review l569 | T4 on device to 1e-6; T6(b) on device == fp64 to 1e-6; P0 IID outputs == host to 1e-6 | I2 | TB4 (W9-10) | REF-05, REF-04 | 2.5 |
| CUD-02 | Register-residency evidence: ptxas -v shows 0 B stack/local and 0 spills at (1,1) (~52 registers, 1024 threads/block) and (1,3) | M | spec 3.1 l527-540; addendum C14 l17-24; review l553 | ptxas report archived; (1,1) launches at 1024 threads/block | I2 | TB4 (W9-10) | CUD-01 | 0.5 |
| CUD-04 | Bucketing by (shape, W), angle gather from the table, C++ batch driver | M | spec 3.1 l541-545, 6.1 l1090-1099; limits Table B B11 | Mixed-shape batch of the P0 IID set == host to 1e-6 | I2 | TB4 (W9-10) | CUD-01 | 1.0 |
| CUD-05 | Layout B warp-per-sentence bent-wire forward, W <= 6 | M | spec 3.1 l546-575; spec 6.1 | T6(c) on device: psi[0], psi[2] == sigma to 1e-6, D = Z | I2 | TB4 (W9-10) | REF-06 | 1.5 |
| CUD-06 | Device readout with D, E-11 certificate and D-floor flag | S | errata E-11 l160-163; spec 6.2 | Device P_c and D == host to 1e-6; flags identical | I2 | TB4 (W9-10) | CUD-01, ERR-04 | 0.5 |
| P0-13 | B3 GRU d = 64 (28.6 KB) and the d = 8 GRU small-state control | S | review l558; addendum 15.4 item 4 | Both trained on the P0 splits; accuracy per split reported | I2 | TB4 (W9-10) | P0-03 | 1.5 |
| CUD-07 | Layout C block-per-sentence (W <= 11) and register-blocked Layout B (W = 7..10) | C | spec 3.1 l546-630; errata E-13 | T6(c) in Layout C; (2,1) bent-wire N TV N (32 amplitudes) == fp64 to 1e-6 | I2 | TB4 (W9-10) | CUD-05 | 2.0 |
| CUD-09 | Layout A training: tensordot reverse rule on device (per-word recurrence, scatter-add into the flat gradient table) | M | review l569; addendum 11.7; errata Sec. B l210-211 | Device gradients == host REF-13 to 1e-6; T24(a) on device | I2 | TB5 (W11-12) | CUD-01, REF-13 | 2.0 |
| CUD-10 | Adam kernel over P_total (5.9), clip 1, eps_D = 1e-7, D histogram and clip-rate logs per epoch; export of the trained model (v1) with round trip | M | spec 5.5, 6.1 l1090-1099, 6.3; errata E-10 l137-146; bridge 8.4 | First 10 steps == host trainer to 1e-6 per parameter (same seed); T10 on the Model A file; size == 32 + 20 V + 2 P_total B | I2 | TB5 (W11-12) | CUD-09, EXP-01 | 1.5 |
| P0-12 | B2 tiny transformer (d = 48, L = 2, 46.2 KB), int8, own C++ autograd | M | review l557; addendum 15.4 item 4 (B2) | Trained on the P0 splits; accuracy per split reported | I2 | TB5 (W11-12) | P0-03 | 2.0 |
| CUD-12 | Circuit adjoint on Layout B with the Sim15 sign check on device; ptxas evidence for all training kernels | S | spec 5.3; spec T7 l1273; addendum C14 | T7 on device: adjoint == FD to 1e-6, unsigned value has the opposite sign; 0 spill loads/stores recorded | I2 | TB5 (W11-12) | CUD-05, REF-12 | 2.0 |
| CUD-14 | Quantisation-aware forward: inference rounding emulated, fp32 gradients | C | spec 4.6 l893-899 | Q1.15-emulated forward in training; accuracy delta reported | I2 | TB5 (W11-12) | ERR-02, CUD-09 | 1.0 |
| CUD-15 | GPU throughput per shape (sentences per second) | C | companion 13.3 Runtime row | Recorded with GPU model and driver version | I2 | TB5 (W11-12) | CUD-04 | 1.0 |
| QC-01 | Adapter boundary contract: model image (angle table + offset lookup), batch submit, outputs (2^{q_s} + 1 values, D, status), VTCM reservation | M | spec 6.2 l1100-1110, 6.3 l1137-1139; errata E-5 l66-75; spec 3.2 l637-641 | Adapter conformance test: load, submit, read back on the DSP; resident model = angle table + 0 or 4 B per word | I3 | TB6 (W13-14) | FND-03, CUD-10 | 0.5 |
| QC-02 | T9 HVX bring-up through the adapter | M | spec T9 l1283; spec 3.2 l658-663 | T9: vmpy/vasr ramp round trip is the identity; vdelta xor controls k = 0..5 match a lane-id vector; fl(a*b) rounding and subnormals recorded | I3 | TB6 (W13-14) | QC-01 | 1.0 |
| QC-03 | E-9 extension on the device: vector-form coefficient ramp and 65536-code sincos sweep | M | errata E-9 l121; review l195 | Sweep error compared with 3.074e-5 (host); tolerance fixed by a proposed erratum (G9) | I3 | TB6 (W13-14) | QC-02, ERR-06 | 0.5 |
| QC-04 | HVX Q1.15 sentence-per-lane (1,1) fused tensordot: lane-varying Q2.14 coefficient vectors, S_q15 pre-scale, Rt8 = 15 store, int32 accumulate | M | errata E-9 l100-124, E-6; spec 4.7 l902-958; bridge 8.1a | Register peak <= 29 of 32 (tensordot 24, CRz 27, Rx 28-29), 0 spills; WS = 10 <= 11 | I3 | TB6 (W13-14) | QC-02, ERR-02 | 2.0 |
| QC-05 | On-device accuracy: T6(b) in Q1.15 and the device parts of T8 | M | spec T6, T8; errata E-6 | T6(b) within 2e-3 of fp64; worked-example error of the order of 3.11e-5; no int32 beyond 2^30 | I3 | TB6 (W13-14) | QC-04 | 0.5 |
| QC-06 | D-floor rejection (D < 2^-6) with fp32 fallback and certificate on the Q1.15 path | M | errata E-11 l160-163; spec 6.2 | Every sentence with D < 2^-6 re-run in fp32 and flagged; certificate emitted | I3 | TB6 (W13-14) | QC-05, ERR-04 | 0.5 |
| QC-07 | HVX data-path options: exported Q15 (cos, sin) pairs (flags.bit0 = 1, 0 DV) or Q1.31 polynomial via vmpye/vmpyo (~36 DV per 64 angles, estimate); sorted-batch fallback where vgather (v65+) is absent | S | errata E-9 l112-114; spec 6.3 l1132 | Option recorded; T10 with flags.bit0 = 1; device T4 within 2e-3; sorted-batch output == vgather output bit for bit | I3 | TB6 (W13-14) | QC-04 | 1.5 |
| QC-08 | Register/spill report and measured cycles as a C14 target measurement; settled bridge Sec. 10 B items moved to a dated note | S | bridge 8.7 step 4, 8.1a step 4 | Dated measurement note naming part and toolchain | I3 | TB6 (W13-14) | QC-04 | 0.5 |
| QC-10 | fp16-store path on v68+ with hvx-ieee-fp (25-29 registers) | C | errata E-9 l115-116; spec 3.2 l643-654 | T6 fp16-store within 3e-3; FTZ behaviour recorded | I3 | TB6 (W13-14) | QC-04 | 1.0 |
| QC-11 | HMX / Cloud AI 100 probe: one 32x32 fp16 tile against fp64 (only if the adapter exposes HMX) | C | spec 3.3 l719-739; bridge Sec. 10 C | Tile error recorded; never used for probability sums | I3 | TB6 (W13-14) | QC-01 | 1.0 |
| QC-12 | Model B (1,3) Q1.15 inference on HVX: layout selection (gap G31) and kernel | M | review l552-553; spec 3.2 l709-718; this plan G31 | 52-amplitude working set (208 B Q1.15) measured; outputs within 2e-3 of fp64 | I3 | TB7 (W15-16) | QC-04 | 1.5 |
| QC-13 | Full P0 test sets through the adapter end to end (word ids in; 2^{q_s} + 1 outputs, D and status out) against the fp32 reference | M | review l570-571; errata E-11, E-4 (c); spec 6.3 l1138-1139 | Classification agreement reported; max abs(delta P_c) <= certificate per sentence; fallback rate reported; traffic tens of bytes in and 12 B out at (1,1) (spec l1166) | I3 | TB7 (W15-16) | QC-06, QC-12 | 2.0 |
| QC-14 | Measured state bytes and counted ops per sentence on the DSP | M | review l559-560, l563 | Working set <= 128 B (Model A) and <= 416 B (Model B) (falsifier F3, review l563) | I3 | TB7 (W15-16) | QC-12 | 0.5 |
| QC-16 | Adreno fiber-per-sentence fused tensordot (1,1), fp16-store / fp32-compute, through the adapter, with compiler register/spill report | S | spec 3.4 l741-757; bridge Sec. 10 D | T6 in fp16 storage within 3e-3; 0 spills (spills go to system memory) | I3 | TB7 (W15-16) | QC-01 | 2.0 |
| QC-18 | HVX amplitude-per-lane (2,1) kernel with 5 vror + vadd reductions | C | spec 3.2 l714-718; review l198 | (2,1) N TV N within 2e-3 of fp64 | I3 | TB7 (W15-16) | QC-04 | 1.5 |
| P0-14 | 200 hand-written paraphrases authored by someone other than the engineer | M | review l556 | 200 items with author and date; parse-FAIL rate computed | I4 | TB8 (W17-18) | P0-04 | 1.0 |
| P0-15 | 5-seed training of Models A and B on CUDA, >= 3 restarts per seed | M | review l557; errata E-10 l141-142 | Accuracy per split per seed; D histograms at init and after training | I4 | TB8 (W17-18) | CUD-10 | 1.0 |
| P0-16 | 5-seed training of B1, B2, B3, the d = 8 GRU and the direct-factor model; int8 export | M | review l557-559 | Accuracy per split per seed; bytes per model (B1 528 B, B2 46.2 KB, B3 28.6 KB) | I4 | TB8 (W17-18) | P0-06, P0-08, P0-12, P0-13 | 1.0 |
| P0-17 | P0 metrics table: accuracy on IID / role-swap / triple-holdout, parse-FAIL rate, state bytes, ops, registers/spills, D histogram and clip rate | M | review l559-561 | Every metric populated for every model and seed | I4 | TB8 (W17-18) | P0-15, P0-16, QC-14 | 1.0 |
| P0-18 | Gate G4 falsification decision record | M | review l561-563 | Decision per Sec. 5.4 of this plan, signed by the Business Sponsor (or the interim) | I4 | TB8 (W17-18) | P0-17 | 0.5 |
| P0-19 | Uncertainty across seeds for every predeclared threshold | S | companion 13.5 item 4 c742 | Mean, sd and interval reported beside every threshold | I4 | TB8 (W17-18) | P0-17 | 0.5 |
| P0-20 | Ablations: real vs complex (Sim15 / ansatz 4, flags.bit2) and exported Q1.15 model including fallback vs the fp32 reference | S | companion 8.4 c451, c454; spec C10 | Accuracy at equal bytes; accuracy delta and fallback rate reported | I4 | TB8 (W17-18) | P0-15, QC-13 | 1.5 |
| P0-22 | Further benchmarks: MC (130) of Lorenz et al. on the six shapes; closed-domain intent set (8-32 intents) at (1,3) / (1,5) | C | addendum 15.4 items 1, 3 l1614-1624; 15.3(a) | Accuracy, parse-FAIL, bytes and ops reported; literature figures stay UNVERIFIED (bridge Sec. 10 J) | I4 | TB8 (W17-18) | P0-15 | 2.0 |
| DEP-01 | Evidence package: frozen grammar and lexicon, manifests, seeds, configs, fixtures with expected outputs, compiler versions, spill reports | M | companion 16.1 c895-910; bridge G25 | Package reproduces every P0 number from a clean checkout | I4 | TB9 (W19-20) | P0-17 | 1.5 |
| DEP-02 | One documented command for reference checks and a separate command for the P0 demonstration | M | companion 16.1 c897-899; bridge G10 | Both commands run on a clean machine with meaningful exit codes | I4 | TB9 (W19-20) | TST-01 | 1.0 |
| DEP-03 | Negative-result record | M | companion 16.3 c918-921; addendum 15.4 item 5 | Every failed threshold and unsupported case listed with its cause class | I4 | TB9 (W19-20) | P0-18 | 0.5 |
| DEP-04 | Document maintenance after P0: dated measurement notes for settled UNVERIFIED items; erratum proposal for the T9 tolerance (G9) | M | bridge 8.7 step 4, 8.8, Sec. 10 | Bridge re-issued; manifest validates with jq | I4 | TB9 (W19-20) | QC-08 | 1.0 |
| DEP-05 | Increment deployment: Project Review Report (increment) and Benefits Enablement | M | this plan Secs. 3.5, 12 | Report filed; measured values of benefits BN-1..BN-9 recorded | I4 | TB9 (W19-20) | DEP-01 | 0.5 |
| DEP-06 | Retabulate spec Sec. 4.6 right-hand columns under E-11 (G28) with a C program; erratum drafted | S | errata E-11 l149-165; bridge G28 | Two independent programs agree | I4 | TB9 (W19-20) | ERR-04 | 1.0 |
| DEP-07 | Output status record struct (G20): structural validity, task score, numerical validity with certificate, approximation status, fallback used | S | bridge G20; companion 7.6 c410-414 | Struct in the adapter header; every output carries it | I4 | TB9 (W19-20) | QC-13 | 1.0 |
| DEP-08 | Export-field proposals as a C15 version bump: ring order and closing H (G2, G16); r, chi, readout, grammar version, checksum (G19) | C | bridge G2, G16, G19; spec C10 l99; review l402 | Proposals logged in bridge Sec. 12 | I4 | TB9 (W19-20) | EXP-01 | 1.0 |
| DEP-10 | Tractability-contract record for the P0 profile (G18) | C | companion 6.2 c298-308; bridge G18 | Seven fields populated with measured values | I4 | TB9 (W19-20) | P0-17 | 1.0 |
| GRM-01 | Chart recogniser (11.4)-(11.5): saturating counts, first / k-th parse, accept, accept_prefix; host or DSP scalar | M | addendum 11.1 l80-140, Sec. 18 item 12; errata E-12 | T11: every 11.2 row exact; "Alice sleeps and dreams" -> (2,3)(1,4)(0,5)(8,9)(7,10), s at 6; 0 false accepts/rejects on 1e5 strings | I5 | TB10 (W21-22) | P0-04 | 2.0 |
| GRM-02 | E-12 full regression (chart half) | M | errata E-12 l186-188 | Unique parse (0,1)(4,5)(3,6)(7,8), s at 2 for both counterexamples; count == (m-1)(m+1)(m+3)/24 for m = 5..197 | I5 | TB10 (W21-22) | GRM-01 | 0.5 |
| GRM-03 | Sec. 11.2 lexicon over {N, S, I, Q} (C15); subject relative n^r n s^l n, object relative n^r n n^ll s^l | M | addendum 11.2 l141-189, C15 l26-36; bridge 8.2 | Every row returns its listed cups and survivor via GRM-01 | I5 | TB10 (W21-22) | GRM-01 | 1.0 |
| GRM-04 | build_tree: word graph, spanning tree, cycle wrapper, CHAIN nodes, post-order by decreasing peak (11.18'); 14 B Node; nchild <= 4 | M | addendum 11.4 l253-401, C16 l38-56, Sec. 18 item 13 | Tree for the T12 sentence matches the addendum; asserts fire on nchild > 4 | I5 | TB10 (W21-22) | GRM-03 | 1.5 |
| GRM-05 | Ambiguity accumulators (11.25) / (11.26) and host chart memory measurement | S | addendum 11.6, l105; bridge G26 | Two-parse string: kth(0) != kth(1) (T11); accumulators 16 B / 32 B; chart 1.375 m^2 B: m = 92 -> 11,638 B, m = 256 -> 90,112 B | I5 | TB10 (W21-22) | GRM-01 | 1.5 |
| GRM-07 | Greedy kept as a six-shape pre-check only, with dispatch to the chart | S | errata E-12; addendum 11.1 | Greedy used only when every word is in the six-shape table | I5 | TB10 (W21-22) | GRM-01 | 0.5 |
| GRM-08 | Rule-based POS tagger front end (size and accuracy UNVERIFIED) | C | addendum 11.6; bridge Sec. 10 H | Tagger bytes and accuracy on a public set measured | I5 | TB10 (W21-22) | GRM-03 | 2.0 |
| GRM-09 | Node ops: tensordot per (type string, cmask, pmask), spider loop, chain matmul, Delta | M | addendum 11.4 eval_tree, (11.14)-(11.17'), Sec. 18 item 14 | T12: Z = 0.13192995, p = [0.33023573, 0.66976427], Loss(y=0) = 1.10794854; whom Z = 0.15647590; T13: "Alice and Bob sleep" Z = 0.04209530; T14 to 1e-15 | I5 | TB11 (W23-24) | GRM-04 | 2.5 |
| GRM-10 | Tree adjoint for arbitrary trees: sign_k = +1, spider and chain adjoints, M_train accounting (11.31) | M | addendum 11.7 l501-571, Sec. 18 item 21 | T24(b)-(e): T12 sentence gradients as listed, M_train = 50; (d) L = 0.68668626; omitting scatter-add FAILs | I5 | TB11 (W23-24) | GRM-09, REF-13 | 2.0 |
| GRM-11 | T22 scheduler memory program (none on disk) | M | addendum T22 l1876; bridge G10, conflict row 36 | Two relative clauses 16 / 48 (12 / 40 in place); believes sentence 14 / 44; streaming 20 / 80 | I5 | TB11 (W23-24) | GRM-04 | 0.5 |
| GRM-12 | (2,1) relative-clause fp64 vector (none exists): extend addendum/t18.c, append to addendum Sec. 18 | S | bridge G10, 8.2 step 3 | Brute force over the product state agrees to 1e-12; addendum re-hashed | I5 | TB11 (W23-24) | GRM-09 | 1.0 |
| GRM-13 | CUDA node-op kernels for the relative clause at (1,1) and (2,1) | S | addendum Sec. 16 a1682-1683 | 0 spills; peak 14 (10) / 44 (36) amplitudes | I5 | TB11 (W23-24) | GRM-09, CUD-01 | 1.0 |
| GRM-14 | HVX Q1.15 node op for the (1,1) relative clause | C | addendum Sec. 16; errata E-9 | Within 2e-3 of fp64; register report | I5 | TB11 (W23-24) | GRM-09, QC-04 | 1.5 |
| GRM-15 | Nested cup-pairing test vector at q >= 2 (flags.bit1 = 1) | C | bridge G5; spec C8 l73-80 | fp64 vector produced by two programs | I5 | TB11 (W23-24) | REF-06 | 0.5 |
| GRM-16 | Export v+1 (C15): base codes 2 = i, 3 = q; header q_i, q_q; spider marker 0xFE; version + 1 | M | addendum C15 l26-36, Sec. 18 item 15 | T10 extended: round trip of a relative-clause model; version incremented | I5 | TB12 (W25-26) | EXP-01, GRM-03 | 1.0 |
| GRM-17 | Circuit-form colouring (11.22): transparent spiders, Delta ancillas, Bell fallback | M | addendum 11.5 l402-451, Sec. 18 item 16 | T18: "Alice who loves Bob sleeps" states {loves, sleeps}, effects {Alice, Bob}, W_hw = 3; "Alice and Bob sleep" W_hw = 3; triangle has one Bell cup | I5 | TB12 (W25-26) | GRM-04 | 1.0 |
| GRM-18 | CUDA training of Sec. 11 sentences via the tree adjoint | M | addendum 11.7; Sec. 16 training rows | Device gradients == host T24(b) to 1e-6; M_train 400 B for the T12 sentence | I5 | TB12 (W25-26) | GRM-10, CUD-09 | 1.5 |
| GRM-19 | Parse-FAIL rate of the 200 P0 paraphrases with the Sec. 11 lexicon | M | addendum 15.4 items 1, 5 | Rate reported beside the expected 10-30 % (a hypothesis) | I5 | TB12 (W25-26) | GRM-03, P0-14 | 0.5 |
| GRM-20 | Grammar evaluation: RP (105) relative-pronoun benchmark (readout on the surviving n wire) and the grammar ablation | S | addendum 15.4 item 3 l1624-1627; companion 8.4 c453 | Accuracy reported with dataset provenance; coverage and accuracy against construction count | I5 | TB12 (W25-26) | GRM-18, GRM-19 | 2.0 |
| GRM-22 | Proposals: canonical Sec. 11 circuit qubit numbering (G13), tree parameter order (G12), binary encoding of cups[] / tree[] (G4) | C | bridge G4, G12, G13; addendum C16 | Proposals logged; cups[] / tree[] round-trip test passes | I5 | TB12 (W25-26) | GRM-17, GRM-04 | 2.0 |
| CAP-01 | MPO verb two-stage contraction (12.14) with streamed cores, and the mixed-radix odometer (12.16') | M | addendum 12.5 l755-822, (12.16') l807-818, Sec. 18 item 17 | T17: V = [21, 0, 36, 3, 1, -4, 6, -3], u = (7,0), w = (6,2), sigma = (70, 126) dense and streamed, B read once; T16: (3,3,3): (2,1,0) -> 5, (1,2,2) -> 25, 17 -> (2,2,1); (3,2,3): 13 -> (1,0,2); odometer 0..17 in order | I6 | TB13 (W27-28) | REF-05 | 2.0 |
| CAP-03 | Isometry generation via U_A^T / U_C (12.15) and operator-norm projection (12.16) by power iteration | M | addendum 12.5 (12.15)-(12.16) | Operator norm <= 1 after projection (to 1e-6 against an fp64 SVD) | I6 | TB13 (W27-28) | CAP-01 | 1.5 |
| CAP-04 | (4,2) precision study | M | addendum 12.3 l672-740; T23 l1878; errata E-11 | T23: mean Z 0.25 / 0.0625 / 0.0039 +- 10 % over 400 triples; Q1.15 (4,2) vs fp32 <= 2.5e-2 in p (loose ceiling) | I6 | TB13 (W27-28) | CAP-01, ERR-02 | 1.0 |
| CAP-05 | Retabulate addendum a708-709, the T23 threshold and the 14.2 clause table under E-11 (G28) | S | bridge G28, conflict row 39 | Two programs agree on 1.25e-2 and p_min 1.05e-4 or correct them; erratum drafted | I6 | TB13 (W27-28) | CAP-04 | 1.0 |
| CAP-06 | Training-dependent Z: D histograms per epoch in a (4,2) run | S | bridge Sec. 10 K; puremath App. B item 15 | Histograms logged; finding on whether training raises Z above 2^{-2 q_n} | I6 | TB13 (W27-28) | CAP-04 | 1.0 |
| CAP-07 | Ansatz capacity items: Jacobian ranks at Q = 4, 5 (sufficiency of L_sat) and a defined ansatz_id 5 with a test vector (G3) | C | addendum l586, l1201-1203; puremath App. B item 33; bridge Sec. 10 G, G3 | Ranks recorded against dof 2^{Q+1} - 2; ansatz 5 gate list, P_w, transpose rule and fp64 vector | I6 | TB13 (W27-28) | REF-04 | 2.0 |
| CAP-09 | Angle generator (13.1)-(13.2): safe cast, integer path, FNV-1a, mix32 zero table, integer mean, type classifier with FAIL-retry | M | addendum 13.1 l881-949, Sec. 18 item 18 | T19: -1.0 -> 60321; 0.8 -> 4172; 13.0 -> 2261; NaN -> 0; acc = 1000 -> 4172 +- 1; mix32(0) = 0x92CA2F0E; FNV-1a("<the>") = 0xFCF0F1C0 | I6 | TB14 (W29-30) | EXP-01 | 2.0 |
| CAP-10 | Generator export block (13.7) with flags.bit3, class records and loader checks | M | addendum 13.7 l1123-1166; C15 | Loader rejects a Cc mismatch; T19 vectors pass; (1,1) IQP L = 2, m = 32 block = 808 B | I6 | TB14 (W29-30) | CAP-09, GRM-16 | 1.0 |
| CAP-11 | Non-zero generator bias at init | M | errata E-10 l147; addendum (13.1) | Zero-bias init shown to sit on the theta = 0 saddle (mixed derivative -0.25); non-zero bias leaves it | I6 | TB14 (W29-30) | CAP-09 | 0.5 |
| CAP-12 | Hybrid gradients by segmented-reduction scatter | M | addendum 13.4 l1045-1069 | Generator gradients == FD to 1e-6 on a (1,1) example | I6 | TB14 (W29-30) | CAP-09, CUD-09 | 1.0 |
| CAP-13 | Hybrid evaluation (head over hashed features vs a linear probe) and hybrid memory rows measured | S | addendum 15.4, Sec. 16 (13.4') rows; bridge Sec. 10 I | Accuracy and bytes for both; zero-table front end 0 B + 429 B at (1,1), m = 32 | I6 | TB14 (W29-30) | CAP-12 | 2.0 |
| CAP-15 | Context gate (13.9) with BPTT | C | addendum 13.2, Sec. 16 context row | 8 KB / 16 KB int8 measured; gradient == FD | I6 | TB14 (W29-30) | CAP-12 | 1.5 |
| CAP-16 | On-device per-token generator pipeline (13.5) | C | addendum 13.5 l1070-1091 | Device k == host k for 1e4 tokens | I6 | TB14 (W29-30) | CAP-09, QC-04 | 0.5 |
| DSC-01 | DisCoCirc build_gates: C17 argument resolution, emission order, pi permutation when i_subj > i_obj, compile-time switch over field pairs | M | addendum C17 l58-70, 14.1 l1174-1243, Sec. 18 item 19 | T20 (write it): wires (0,1) and (1,0) give identical rho_Alice to 1e-12; U_big before U_loves; real Sim15 verb FAILs, Sim14 passes | I7 | TB15 (W31-32) | GRM-09 | 2.0 |
| DSC-02 | RDM readout (14.19): fp32 accumulation, ket row / bra column, division by Tr rho | M | addendum (14.19) l1402-1403; bridge 8.9 | 128 B accumulator at q_n = 2; limits T-W3 at q_n = 1 reproduced | I7 | TB15 (W31-32) | DSC-01 | 1.0 |
| DSC-03 | Spectral functionals (limits A2): d = 2 closed form, d = 4 cyclic Jacobi with 3 sweeps, fp32 clamp lam <= 1e-7, log base stated | M | limits A2 l332-480; bridge conflict rows 41-42 | T-SF1..T-SF4: lam 0.93996159 / 0.06003841, S = 0.22707320 nat, purity 0.88713240, F_sq 0.93359620, T 0.09984988 | I7 | TB15 (W31-32) | DSC-02 | 1.5 |
| DSC-04 | All-wires partial trace in one pass (limits A4) | M | limits A4 l626-793; bridge 8.9 | T-W4: eig 0.97766824 / 0.02233176 / 0 / 0, S = 0.15433977 bit, purity 0.95633390, I = 0.30867954 bit; dense 16x16 path == gate-by-gate to 1e-6 | I7 | TB15 (W31-32) | DSC-02 | 1.5 |
| DSC-05 | coref_resolve interface (string-equality coreference) | S | addendum 14.5 l1378-1397; bridge Sec. 10 H | Interface and fixture; accuracy recorded as UNVERIFIED | I7 | TB15 (W31-32) | DSC-01 | 1.0 |
| DSC-06 | Proposals: eps for fidelity readouts (G11) and one packed-Hermitian layout (G7) | S | addendum 14.6 l1425; bridge G7, G11 | Proposals with a derivation program | I7 | TB15 (W31-32) | DSC-03 | 1.0 |
| DSC-07 | Non-unitary word maps on density matrices (limits A1) | C | limits A1 l173-331 | A1-T1..T5b: fuzz diag(0.5, 0.125), T = 0.625; adjoint P = 0.8520079519 | I7 | TB15 (W31-32) | DSC-02 | 2.0 |
| DSC-08 | MPS: two-site update, rank guard S_tol = 1e-6, sorted one-sided Jacobi incl. m < n, QR centre moves, log Z carry | M | addendum 14.3 l1270-1355, Sec. 18 item 20 | T15: S = [3, 2.2360679775, 2.2360679775, 1], residuals <= 2e-15; rank guard S = [1,0,0,0] -> chi_i = 1, no NaN; T21: chi = 8, N = 6, 30 U(4) gates, error <= 1e-10 | I7 | TB16 (W33-34) | DSC-01 | 3.0 |
| DSC-09 | MPS single- and two-wire RDMs (14.19') and Uhlmann fidelity by two Hermitian Jacobi calls | M | addendum (14.19') l1405-1415 | Two-wire RDM from the MPS == exact RDM to 1e-6 in fp32 at exact chi | I7 | TB16 (W33-34) | DSC-08, DSC-04 | 1.0 |
| DSC-10 | Gram matrix, InfoNCE and slot scoring (limits A3) | S | limits A3 l481-625 | A3-gram: Z_0 = 0.14026553, Z_1 = 0.16045040, F_01 = 0.97273229, InfoNCE L = 0.53396114 | I7 | TB16 (W33-34) | REF-09 | 1.5 |
| DSC-11 | MPS memory in the 8 KB slice regime measured | S | addendum Sec. 16 MPS rows | N = 32, chi = 4: 7,488 B fp32 / 3,744 B fp16 + 1.0 KB fp32 scratch | I7 | TB16 (W33-34) | DSC-08 | 0.5 |
| DSC-12 | DisCoCirc higher-order boxes (limits A5) | C | limits A5 l794-952 | A5 Tests A-H: p(Claire = 1) = 0.221700; hedge Z = 0.526433 | I7 | TB16 (W33-34) | DSC-01 | 1.5 |
| DSC-13 | Postselection norm as a plausibility score (limits Table B row B1) | C | limits Table B B1; limits Theme B l1369-1468 | B-parse: parse 0 Z = 0.07915003, parse 1 Z = 0.05651013; q = [0.58344344, 0.41655656] | I7 | TB16 (W33-34) | GRM-05 | 0.5 |
| DSC-14 | Length / productivity split: train stories with <= 4 actors, test 8-16 | M | addendum 15.4 item 2(iii) l1618-1621 | Generalisation and in-distribution accuracy over 5 seeds (outcome is a hypothesis) | I7 | TB17 (W35-36) | DSC-08 | 2.0 |
| DSC-15 | Programme evidence package updated with the I5-I7 numbers; discourse increment deployment record; Benefits Assessment inputs | M | companion 16.1; this plan Secs. 3.5, 12 | Package reproduces every I5-I7 number from a clean checkout; record filed | I7 | TB17 (W35-36) | DSC-14 | 1.5 |
| DSC-17 | DisCoCirc text register on device: N = 4 nouns, q_n = 1 (16 amplitudes, 128 / 64 B) on CUDA and HVX | S | addendum Sec. 16 DisCoCirc rows; 14.1 l1229-1236 | Within 1e-6 (fp32) and 2e-3 (Q1.15) of fp64 | I7 | TB17 (W35-36) | DSC-01, QC-04 | 1.5 |
| DSC-18 | Entropy gradient (limits A4-grad) | S | limits A4 l737-780 | dS_0/dtheta_CRz = 0.42460500 | I7 | TB17 (W35-36) | DSC-04 | 0.5 |
| DSC-19 | bAbI-derived QDisCoCirc QA set | C | addendum 15.4 item 3 | Accuracy reported; dataset provenance recorded | I7 | TB17 (W35-36) | DSC-08 | 1.0 |
| DSC-20 | Exact second-order training diagnostics (limits A6) | C | limits A6 l953-1100 | A6-et: J[0][7] = 0; F_cl eigenvalue 1.687955; E[D] = 0.2500; DLA dim 13 of 63 | I7 | TB17 (W35-36) | REF-12 | 1.0 |

Cross-cutting non-functional requirements (effort carried inside the definition of done and FND-04) and the
Won't-this-time list:

| ID | Requirement | MoSCoW | Source | Acceptance criterion / disposition | Inc | TB | Deps | Effort (d) |
|---|---|---|---|---|---|---|---|---|
| NFR-01 | No Python anywhere in product or tooling; tooling is C / C++ / Mojo, shell, awk, jq, CMake or Bazel, CUDA and Hexagon SDK tools | M | bridge header; this plan header | CI-NOPY green at every close-out | all | all | FND-04 | 0 (in FND-04) |
| NFR-02 | Plain ASCII; documents identified by SHA-256; baseline spec byte-identical, changed only by appended errata | M | bridge header, R1, R13, 8.8; errata l3-8 | CI-ASCII and CI-HASH green at every close-out | all | all | FND-04 | 0 (in FND-04) |
| NFR-03 | Numerical hard rules: no fp16 probability accumulation; no tensor-core or HMX probability sums; no mid-circuit renormalisation (log sums, divide once with the Z_min / eps_D guards); angles stored as uint16 half-angle turns, wrapped only at export and only mod 4 pi; never fp16 angles | M | spec C12 l110-113 with errata E-8; spec 3.3 l738; spec 4.5 l859-862; spec C5 l39-44, 5.6 l1076-1087, l792 | Review checklist item on every kernel; T8 (E-7 form), T10, T7 periodicity | all | all | - | 0 (in DoD) |
| NFR-04 | Conventions C1-C17 obeyed; every emitted gate tagged with its form (circuit, tree, DisCoCirc, doubled register) | M | spec Sec. 0 l11-142; addendum C14-C17 l15-70; bridge R2, R11 | T2, T7 (sign_k = -1 circuit), T24 (sign_k = +1 tree), T20 | all | all | - | 0 (in DoD) |
| NFR-05 | Every kernel checked against the fp64 reference at fp32 1e-6, fp16-store 3e-3, Q1.15 2e-3; residency and cycle claims only as compiler-verified or measured targets | M | spec l1258; addendum l1820, C14 l17-24; bridge R7 | Definition of done (Sec. 7.5) | all | all | TST-01 | 0 (in DoD) |
| WNT-01 | Accounting semantics, ACDOCA data contract, grammar-derived accounting composition, journal review, NL search over postings, reconciliation | W | companion Secs. 10-12.5 c500-649; review l573-576; bridge Sec. 11 Deferred | Not built; revisit only after G4 passes AND a quantum-form component beats a direct-factor model at matched bytes (c450) | -- | -- | G4 | 0 |
| WNT-02 | Invoice-to-Payment and other distributed process profiles; distributed coordination and agent action contracts | W | companion 12.6-12.8 c650-691, 13.4 c727-733, 13.6 c748-755; review l573 | Not built | -- | -- | G4 | 0 |
| WNT-03 | Temporal context layer (LNN / CfC) | W | companion Sec. 14 c756-856; review l573 | Not built | -- | -- | G4 | 0 |
| WNT-04 | Companion 13.4 profiles other than P0 (64 KiB command interpreter, phone similarity, journal scorer); MCU / no-FPU / 1 mW targets | W | companion 13.4 c729-731; bridge G24 | Not built | -- | -- | -- | 0 |
| WNT-05 | lambeq / DisCoPy weight import and convention matching (would require running Python tooling) | W | spec Sec. 10 item 1 l1293; bridge Sec. 10 A | Not built; training from scratch is unaffected (spec l1296) | -- | -- | -- | 0 |
| WNT-06 | Pure fp16 / bf16 arithmetic paths; N(0, 0.1^2) initialisation | W | spec 4.6 l893-896; errata E-10 l126-147 | Not built; asserted absent in code review | -- | -- | -- | 0 |
| WNT-07 | Depth-first-search parser extension; monolithic (2,1) forms with adjectives | W | errata E-12 l167-192; spec Sec. 7 l1177-1179 | Not built | -- | -- | -- | 0 |
| WNT-08 | On-device SPSA fine-tuning; frozen-transformer front end (22-110 MB encoder, UNVERIFIED) | W | spec 5.4 l1044-1053, 6.2 l1108-1110; addendum 13.6 l1092-1122 | Not built | -- | -- | -- | 0 |
| WNT-09 | limits Theme B beyond row B1, limits A5-A8 beyond the Coulds listed, and application uses of addendum 13.3, 13.6, 14.5-14.7 beyond the Sec. 18 checklist and the 15.4 evaluation plan | W | limits Tables A/B l35-124; bridge Sec. 11 Deferred | Not built; candidates for a later programme | -- | -- | G4 | 0 |

### 4.3 Effort and MoSCoW percentages

Per timebox (computed with awk from the PRL source; capacity assumption 10 ideal days per timebox, Sec. 10.2):

| TB | Weeks | Inc | Items | Must (d) | Should (d) | Could (d) | Total (d) | Must % | Should % | Could % |
|---|---|---|---|---|---|---|---|---|---|---|
| TB0 | 1-2 | I0 | 7 | 5.5 | 2.0 | 2.0 | 9.5 | 58 | 21 | 21 |
| TB1 | 3-4 | I1 | 12 | 6.0 | 2.0 | 2.0 | 10.0 | 60 | 20 | 20 |
| TB2 | 5-6 | I1 | 10 | 6.0 | 2.0 | 2.0 | 10.0 | 60 | 20 | 20 |
| TB3 | 7-8 | I1 | 12 | 6.0 | 2.0 | 2.0 | 10.0 | 60 | 20 | 20 |
| TB4 | 9-10 | I2 | 7 | 5.5 | 2.0 | 2.0 | 9.5 | 58 | 21 | 21 |
| TB5 | 11-12 | I2 | 6 | 5.5 | 2.0 | 2.0 | 9.5 | 58 | 21 | 21 |
| TB6 | 13-14 | I3 | 10 | 5.0 | 2.0 | 2.0 | 9.0 | 56 | 22 | 22 |
| TB7 | 15-16 | I3 | 5 | 4.0 | 2.0 | 1.5 | 7.5 | 53 | 27 | 20 |
| TB8 | 17-18 | I4 | 8 | 4.5 | 2.0 | 2.0 | 8.5 | 53 | 24 | 24 |
| TB9 | 19-20 | I4 | 9 | 4.5 | 2.0 | 2.0 | 8.5 | 53 | 24 | 24 |
| TB10 | 21-22 | I5 | 7 | 5.0 | 2.0 | 2.0 | 9.0 | 56 | 22 | 22 |
| TB11 | 23-24 | I5 | 7 | 5.0 | 2.0 | 2.0 | 9.0 | 56 | 22 | 22 |
| TB12 | 25-26 | I5 | 6 | 4.0 | 2.0 | 2.0 | 8.0 | 50 | 25 | 25 |
| TB13 | 27-28 | I6 | 6 | 4.5 | 2.0 | 2.0 | 8.5 | 53 | 24 | 24 |
| TB14 | 29-30 | I6 | 7 | 4.5 | 2.0 | 2.0 | 8.5 | 53 | 24 | 24 |
| TB15 | 31-32 | I7 | 7 | 6.0 | 2.0 | 2.0 | 10.0 | 60 | 20 | 20 |
| TB16 | 33-34 | I7 | 6 | 4.0 | 2.0 | 2.0 | 8.0 | 50 | 25 | 25 |
| TB17 | 35-36 | I7 | 6 | 3.5 | 2.0 | 2.0 | 7.5 | 47 | 27 | 27 |

Per increment and for the project:

| Increment | Items | Must (d) | Should (d) | Could (d) | Total (d) | Must % | Should % | Could % |
|---|---|---|---|---|---|---|---|---|
| I0 | 7 | 5.5 | 2.0 | 2.0 | 9.5 | 58 | 21 | 21 |
| I1 | 34 | 18.0 | 6.0 | 6.0 | 30.0 | 60 | 20 | 20 |
| I2 | 13 | 11.0 | 4.0 | 4.0 | 19.0 | 58 | 21 | 21 |
| I3 | 15 | 9.0 | 4.0 | 3.5 | 16.5 | 55 | 24 | 21 |
| I4 | 17 | 9.0 | 4.0 | 4.0 | 17.0 | 53 | 24 | 24 |
| I5 | 20 | 14.0 | 6.0 | 6.0 | 26.0 | 54 | 23 | 23 |
| I6 | 13 | 9.0 | 4.0 | 4.0 | 17.0 | 53 | 24 | 24 |
| I7 | 19 | 13.5 | 6.0 | 6.0 | 25.5 | 53 | 24 | 24 |
| Project | 138 | 89.0 | 36.0 | 35.5 | 160.5 | 55 | 22 | 22 |

Reading: every timebox keeps Musts at or below 60 % of its estimated effort; the project total is 55 % Must,
22 % Should, 22 % Could. Timeboxes below 10 ideal days (TB7, TB12, TB16, TB17 at 7.5-8.0) are deliberately light:
TB7 and TB16 carry the highest technical uncertainty (device layout gap G31; MPS), TB12 and TB17 close an
increment and absorb re-planned Shoulds.

PRL size: 138 schedulable requirements (by count: 77 Must, 34 Should, 27 Could), plus 5 cross-cutting non-functional Musts and 9 Won't-this-time items.
Total rows in the PRL: 152.


## 5. Increments, timeboxes and go/no-go gates

### 5.1 Increments

Each increment ends in a deployable, demonstrable outcome (Sec. 3.5). Adjustments to the suggested increments
and why:
  (a) I1 also carries the P0 DATA, HOST TRAINING and an EARLY READ (gate G1, Week 8). The review sizes P0 as CPU
      training with a CUDA GPU only for the residency check (review l564); running the accuracy part on the host
      as soon as the reference trainer exists lets the programme fail fast (DSDM principles 1 and 5) before 8
      weeks of device work.
  (b) The B2 transformer baseline is a Must of I2 (TB5), not I4, because falsifier F1 compares against it.
  (c) I4 is two timeboxes: formal runs and the decision (TB8), then consolidation and Deployment of the P0
      evidence (TB9), so that the decision is never rushed into a close-out day.
  (d) I5-I7 exist only if G4 passes; their order follows the dependency chain of the addendum (Sec. 11 node ops
      are needed by DisCoCirc build_gates, addendum Sec. 18 item 19).

| Increment | Weeks | Timeboxes | Deployable / demonstrable outcome | Gate |
|---|---|---|---|---|
| I0 Foundations | 1-2 | TB0 | Feasibility and Foundations products; CI with no-Python / ASCII / hash-pin jobs | G0 |
| I1 fp64 reference, harness, errata, P0 early read | 3-8 | TB1-TB3 | fp64 C reference for spec Sec. 9 items 1-10; assertion harness running T1-T8, T10, T-EZ and the errata tests; host trainer; P0 data; early read | G1 |
| I2 CUDA training path for P0 at (1,1) and (1,3) | 9-12 | TB4-TB5 | Layout A fused tensordot and training, Layout B bent-wire, Adam, export; B2 baseline | G2 |
| I3 Qualcomm inference through the adapter | 13-16 | TB6-TB7 | HVX Q1.15 sentence-per-lane (1,1) with the E-9 register budget; (1,3) on HVX; Adreno (Should); accuracy vs fp32 reference | G3 |
| I4 P0 execution and falsification decision | 17-20 | TB8-TB9 | P0 result with 5 seeds, all baselines, evidence package, negative-result record | G4 (P0 FALSIFICATION GATE) |
| I5 Grammar extension | 21-26 | TB10-TB12 | Chart parser, build_tree, node ops, spiders, relative clauses, coordination, tree adjoint, export v+1 (T11-T14, T18, T22, T24) | G5 |
| I6 Capacity and hybrid | 27-30 | TB13-TB14 | MPO verbs, mixed radix, (4,2) precision, hashed n-gram angle generator and its export block (T16, T17, T19, T23) | G6 |
| I7 Discourse | 31-36 | TB15-TB17 | DisCoCirc, RDM and spectral readouts (limits A2, A4), MPS (T15, T20, T21, T-W4), length split | G7 |
| Final Deployment | 37-38 | -- | Consolidated release, final Project Review Report, Benefits Assessment plan | Sponsor acceptance |

Timebox count: 18 timeboxes (TB0-TB17) of 2 weeks = 36 weeks, plus a 2-week final Deployment = 38 weeks nominal.
If G4 stops the programme, the project ends at Week 20 after 10 timeboxes (TB0-TB9).

### 5.2 Standard timebox structure (2 weeks = 10 working days)

| DSDM step | Days | What happens (every timebox) |
|---|---|---|
| Kick-off | Day 1 (1-2 h) | Confirm the objective; pull the PRL items listed for the timebox; re-read their acceptance criteria and sources; assign agents (developer session and a separate tester session per item); confirm the agent-usage budget; list the risks to watch. |
| Investigation | Days 1-2 | Read the bridge procedure(s) for the modules touched (bridge Sec. 8) and the cited document lines; the Solution Tester session writes the acceptance tests FIRST from the documents and the reference programs; spikes on unknowns (device facts, toolchain). |
| Refinement | Days 3-8 | Implement and iterate against the tests; daily stand-up (Sec. 11.2); when a Must is at risk, drop Coulds, then Shoulds (Sec. 11.3). |
| Consolidation | Day 9 | All Must tests green in CI; evidence captured (test logs, ptxas / compiler reports, measurement notes); bridge / manifest updates drafted if a document-level fact changed; demo rehearsed. |
| Close-out | Day 10 | Demonstration; Timebox Review Record; MoSCoW renegotiation for the next timebox; retrospective (15 minutes); one-page status report (Sec. 11.5). |

### 5.3 Timebox Review Record (template; every timebox fills all fields plus its specific fields in Sec. 5.5)

| Field | Content |
|---|---|
| TB id, increment, weeks | e.g. TB3, I1, Weeks 7-8 |
| Objective met | yes / partly / no, one sentence |
| PRL items | per id: MoSCoW, delivered (Y/N), estimate vs actual ideal days |
| Tests | per test id: expected value (from the PRL), measured value, pass/fail, tolerance used |
| Evidence | paths of logs, compiler reports, measurement notes; evidence label per claim (Specified / Derived / Reference-checked / Measured / Hypothesis / Open) |
| UNVERIFIED items touched | bridge Sec. 10 group letter, item, new status, dated note |
| Gaps and conflicts raised | bridge Sec. 11 id or new G-id; Sec. 2.2 conflict row format (topic, a, b, resolution, rule) |
| Errata / addendum proposals | E-14, ... drafted per bridge 8.8, or addendum change ids |
| Agent usage | budget vs consumed; items lost to usage limits |
| Velocity | ideal days delivered (Musts / Shoulds / Coulds) |
| Risks | ids opened, changed, retired (Sec. 8) |
| Decisions | MoSCoW changes (with reason), gate outcome if any |
| Carried forward | ids and target timebox |
| Demo feedback | who attended (roles, vacancies noted), comments |
| Sign-off | Team Leader (interim: engineer); Business Ambassador (VACANCY: note "not reviewed by business") |

### 5.4 Go/no-go gates

| Gate | When | Criteria (all from the documents) | If the criteria fail |
|---|---|---|---|
| G0 Foundations | end Week 2 | PRL baselined, Musts <= 60 % per timebox; CI jobs FND-04 green; hardware access confirmed or fallback chosen; operational definition of "matches" for falsifier F4 fixed (see below) | Stay in Foundations one more week at most; otherwise Sponsor decision |
| G1 P0 early read | end Week 8 | Host training, seed 1, Model A: IID >= 90 % (F2 threshold, review l562); role-swap >= 90 % (predeclared, review l561-562); B1 role-swap = 50 % (review l555, a sanity check of the label function); direct-factor model compared on every split | IID < 90 %: TB4 kick-off becomes a diagnosis (init per errata E-10, >= 3 restarts, L = 2 per review l550); if still < 90 % after that diagnosis, run the formal P0 on the host at once (pull TB8 forward) and take G4 early. Role-swap < 90 % or direct-factor >= Model A everywhere: continue, flag to Sponsor, record in the risk log |
| G2 CUDA | end Week 12 | ptxas -v: 0 spills and 0 B local memory at (1,1) and (1,3) (CUD-02; addendum C14); device == host to 1e-6 (T4, T6(b), T6(c), T24(a) on device); training steps == host (CUD-10) | Redirect training to Layout B/C (spec 6.1) or host; P0 can still run from host training (review l564) |
| G3 Qualcomm | end Week 16 | HVX (1,1) register peak <= 29 of 32 and 0 spills (errata E-9 l108-110); T9 green; T6(b) Q1.15 within 2e-3 of fp64; every P0 test-set sentence within its certificate 2e/(2 sqrt D - e) or routed to fp32 at D < 2^-6 (errata E-11 l160-163); working set measured (QC-14) | P0 device claims restricted to CUDA; HVX facts stay UNVERIFIED (bridge Sec. 10 B); F3 evaluated on the CUDA evidence and so labelled |
| G4 P0 FALSIFICATION GATE | decision end Week 18, record and Deployment by end Week 20 | See below | See below |
| G5 Grammar | end Week 26 | T11, E-12 (chart half), T12, T13, T14, T18, T22, T24(b)-(e) green | I6 / I7 re-planned; DisCoCirc (I7) cannot start without node ops (GRM-09) |
| G6 Capacity / hybrid | end Week 30 | T16, T17, T19, T23 green; generator block loads with the loader checks | I7 proceeds (independent of I6); I6 Shoulds carried |
| G7 Discourse | end Week 36 | T20 (written), T-W3, T-W4, T-SF1..T-SF4, T15, T21 green | Final deployment ships what passed; the rest is recorded as negative or open |

G4, the P0 falsification gate. Inputs: the P0 metrics table (P0-17) over 5 seeds at 4,000 training sentences.
The review's falsifiers (review l562-563), any one of which falsifies P0:

| Id | Falsifier (verbatim content of review l562-563) | Measured by |
|---|---|---|
| F1 | QNLP role-swap accuracy <= B2 role-swap accuracy at 4K training sentences | P0-15, P0-16 |
| F2 | QNLP IID accuracy < 90 % | P0-15 |
| F3 | Measured working set exceeds 128 B (Model A) / 416 B (Model B) | QC-14, CUD-02 |
| F4 | The equal-byte direct-factor model matches QNLP on every split | P0-06, P0-16 |

Predeclared expectations that are NOT falsifiers but are reported against (review l561-562): IID QNLP >= 97 %,
B2 >= 97 %, B1 <= 75 %; role-swap QNLP >= 90 %, B1 = 50 %, B2 55-80 %; triple-holdout QNLP >= 90 %, B2 70-90 %.
Operational definition of "matches" in F4 (not given by the review; must be fixed at G0, before any P0 run, and
written into the ToR; proposal for the Foundations workshop: the difference of the 5-seed mean accuracies is
not larger than the pooled across-seed standard deviation). Fixing it after seeing results is forbidden.

| G4 outcome | Condition | Decision |
|---|---|---|
| PASS | No falsifier fires and the QNLP expectations are met | Proceed to I5; I6 and I7 follow their gates. The c450 condition for limits themes ("a quantum-form component beats a direct-factor model at matched bytes", review l573-574) is recorded as met on the split(s) where QNLP exceeds the direct-factor model. |
| PASS, WEAK | No falsifier fires but a QNLP expectation is missed (e.g. IID between 90 % and 97 %) | Proceed to I5 only; I6 and I7 re-planned at G5; the miss goes into the negative-result record (DEP-03). |
| FALSIFIED by F1 or F2 | The compositional-generalisation claim fails | STOP. No I5-I7. Deploy I4 as a negative result (evidence package, negative-result record per companion 16.3, classifying the failure as profile, parameterisation, numeric format or premise). A single diagnostic timebox is allowed only with Sponsor approval and a NEW predeclared hypothesis. |
| FALSIFIED by F3 only | The "almost zero memory" working-set claim fails on the measured path | Redirect: one timebox to re-plan layouts (in-place forms: 10 / 36 amplitudes, spec l254; addendum (12.11)); re-measure; if still over, STOP the memory claim and report. |
| FALSIFIED by F4 only | The quantum parameterisation adds nothing at equal bytes | The quantum-form line stops: I5 may continue as grammar-derived composition with direct factors (companion 8.4 "Does the PQC parameterization help?" answered no); limits themes in I7 become Won't (c450 not met). |

The G4 record is signed by the Business Sponsor; while that role is vacant it is signed "interim, engineer"
and re-submitted for ratification when a Sponsor is named.

### 5.5 Timebox plans

Each entry gives: timebox id and weeks; objective; what happens in each DSDM step (beyond the standard of Sec.
5.2); the PRL items pulled (MoSCoW, effort) with their acceptance criteria (test ids and expected values);
the demo; the timebox-specific review-record fields (in addition to Sec. 5.3).

#### TB0 -- Feasibility and Foundations (Weeks 1-2) [I0]

Objective: complete Pre-Project, Feasibility and Foundations; baseline the PRL; make CI enforce the hard
constraints; pass G0.
  - Kick-off (Week 1, day 1): Pre-Project ToR draft (Sec. 3.1); confirm the seven pinned hashes (bridge Sec. 1).
  - Investigation (Week 1): Feasibility desk check (verify.c rebuilt and diffed; review Sec. 8 program list
    located); hardware and adapter inventory; Feasibility workshop (Sec. 11.1).
  - Refinement (Week 2, days 1-3): SAD, DAD, MAD drafted from Secs. 6, 7, 11; CI jobs; Foundations workshop:
    MoSCoW, tolerances, the F4 "matches" definition, gates.
  - Consolidation (Week 2, day 4): PRL effort check (Sec. 4.3); risk log imported.
  - Close-out (Week 2, day 5): G0 review; TB1 plan agreed.
PRL items:
  - FND-01 [M, 1.0 d] Terms of Reference, Feasibility Assessment and Business Case outline agreed; P0 falsifiers quoted verbatim. Accept: Foundations sign-off record names the four P0 falsifiers of review l562-563
  - FND-02 [M, 1.0 d] PRL baselined with MoSCoW; Musts <= 60 % of effort per timebox and per increment. Accept: awk effort check (Sec. 4.3 of this plan) shows Must share <= 60 % in every timebox
  - FND-03 [M, 1.5 d] Solution Architecture Definition: module map (spec Sec. 9 items 1-11, addendum Sec. 18 items 12-21) and the adapter boundary header in C. Accept: SAD reviewed at the Foundations workshop; adapter header compiles with cc -std=c99 and nvcc
  - FND-04 [M, 2.0 d] Development Approach Definition: CMake (or Bazel) build, ctest skeleton, CI jobs rejecting any Python file and any non-ASCII byte, and a hash-pin job checking the seven SHA-256 values of bridge Sec. 1. Accept: CI fails on a planted *.py file, a planted non-ASCII byte and a changed pinned document; sha256sum -c passes on all seven
  - FND-06 [S, 1.5 d] Environment readiness: CUDA toolchain + one GPU; Hexagon SDK + the engineer's adapter + a v65+ part (vgather) or the sorted-batch plan. Accept: Toolchain versions recorded; a trivial kernel runs on the GPU and through the adapter on the DSP
  - FND-07 [S, 0.5 d] Risk log seeded from bridge Sec. 10 (A-O), Sec. 11 (G1-G30) and puremath App. B; evidence labels adopted in status reports. Accept: Risk log of this plan Sec. 8 in the tracker; status template carries an evidence label per claim
  - FND-08 [C, 2.0 d] Apply the addendum edits owed by the errata and review as an addendum revision with a new hash pin; time the first agent task against the engineer (velocity calibration). Accept: New addendum hash in bridge Sec. 1 and manifest; every aNNN citation re-checked; calibration hours recorded
Demo: CI rejecting a planted *.py file, a planted non-ASCII byte and a modified pinned document; the PRL effort
check printing <= 60 % Must per timebox.
Review-record specific fields: G0 outcome; hardware inventory (GPU model, Hexagon part and version, adapter
capabilities: VTCM reservation yes/no, vgather available yes/no); F4 "matches" definition as fixed; named or
vacant business roles.

#### TB1 -- fp64 reference kernels and harness (Weeks 3-4) [I1]

Objective: a plain-C fp64 reference for spec Sec. 9 items 3-7 with an assertion harness that reproduces T1-T6.
  - Kick-off: modules M3-M7 and M0; bridge procedure 8.1 (implement a kernel).
  - Investigation: tester session writes T1-T6 assertions from spec l1257-1272 and verify.c output; T2 has no
    program on disk (bridge G10), so its dense reference M is built first (C13 l115-142).
  - Refinement: index helpers, gates by bit tests (C6, never permuted matrices), word states, tensordot with the
    E-13 slot rule, bent-wire with the wire-map table, readout with guards.
  - Consolidation: all Must tests green; verify.c regression job (TST-02) compared.
  - Close-out: demo; record residual digits against verify.c (bridge Sec. 10 O: gcc -O0 x86-64 values).
PRL items:
  - REF-01 [M, 0.5 d] Index helpers i0, i1, i00, compact (C1). Accept: T1: i0 bijective onto {bit_k = 0}, i00(j) >= j, compact(i00(j)) == j for all lo < hi at W = 6
  - REF-02 [M, 1.0 d] Gates Rx, Ry, Rz, H, CNOT, CZ, CRz, CRx defined by bit tests; apply_1q / apply_2q; T2 program written (none exists on disk). Accept: T2: apply == dense M psi at W = 6, both control orientations, lift_lo / lift_hi, fp64 to 1e-12
  - REF-03 [M, 0.5 d] Gate identities including the CRx check that verify.c does not code. Accept: T3: CRz decomposition residual < 1e-15 (verify.c 1.68e-16); CRx = Rx CZ Rx CZ; CNOT sandwich != I asserted; U^T by reversed list == transposed dense for IQP and Sim15
  - REF-04 [M, 1.0 d] Word-state generator for ansatz ids 0-4 with gate-list order C4 and field layout C7. Accept: T4: Alice [0.71847188-0.31582980i, -0.13353070-0.60516052i]; Bob [0.74959627+0.19470917i, 0.03946950-0.63137622i]; loves abs(V[k]) = 0.35355339
  - REF-05 [M, 1.0 d] Fused tensordot over field lists with the E-13 slot rule (one 8 B accumulator). Accept: T5: sigma = [0.17991068-0.41061012i, 0.41557129-0.46260942i], Z = 0.58767549; T6(a),(b): Z = 0.14026553, p = [0.58263882, 0.41736118]
  - REF-06 [M, 1.0 d] Bent-wire circuit form with the Sec. 1.4 wire-map table; postselect-compact in descending order; monolithic Bell-cup path. Accept: T6(c): psi[0], psi[2] == sigma to 1e-16, D = Z; T6(d): P_post = Z/4 = 0.03506638; T7 six wire-map rows == tensordot to 1e-12
  - REF-07 [M, 0.5 d] Readout N_c, D, P_c with Z_min = 1e-12, eps = 1e-9, eps_D = 1e-7, widen-before-square. Accept: T5: p = [0.34197193, 0.65802807]; T6: Loss(y=0) = 0.54018780, Loss(y=1) = 0.87380330; all-zero angles Z = 0.25
  - TST-01 [M, 0.5 d] Assertion-based one-command harness (ctest) at the spec tolerances fp32 1e-6, fp16-store 3e-3, Q1.15 2e-3; every new expected value produced by two separate programs. Accept: ctest runs T1-T6 and exits non-zero on any failure; each new value cites two agreeing programs
  - REF-08 [S, 1.0 d] Matmul form (2.8) and X-only flip-mask relabelling (2.4). Accept: T6(a) via the matmul form == (1.5) to 1e-12; CNOT never treated as a flip mask (spec l470)
  - REF-09 [S, 0.5 d] Fidelity between two sentence states (1.5). Accept: limits A3-gram F_01 = 0.97273229 reproduced in fp64 to 1e-8
  - TST-02 [S, 0.5 d] verify.c regression job: cc -O2 verify.c -lm, printout diffed against spec Sec. 8. Accept: Printout matches Z = 0.14026553 and every Sec. 8 value; verify.c hash checked
  - REF-10 [C, 2.0 d] Alternative reference paths: Q1.31 (int32 state, int64 accumulate) and the early-cup monolithic evaluator. Accept: Per-gate error <= g_Q31(W) = 8.5e-8 at W = 16 (spec l826); T6(d) via early cups with peak 24 amplitudes (spec l459)
Demo: `ctest` run printing T1-T6 with measured vs expected values; Z = 0.14026553 by three evaluation routes
(tensordot, bent-wire, monolithic).
Review-record specific fields: max abs deviation from verify.c per test; T2 coverage (all lo < hi at W = 6, both
orientations); any conflict with a bridge Sec. 2.2 row observed.

#### TB2 -- Gradients, numerics, export, errata tests (Weeks 5-6) [I1]

Objective: complete the reference: circuit adjoint and tensordot reverse rule, Q1.15 emulation, fp16 rules,
export v1, host Adam trainer; errata E-6, E-7, E-8, E-10 tests green.
  - Kick-off: bridge procedures 8.3 (number format), 8.4 (export), 8.5 (train).
  - Investigation: tester session encodes T7, T8 (E-7 form, E-6 endpoint), T10, E-10 values from errata l86,
    l91, l137-147 and spec l1273-1287.
  - Refinement: adjoint on the unfused gate list (review l320-321); sign_k by form (R11); pre-scale S_q15 on the
    tensordot path only; export offsets per bridge Sec. 3.4.
  - Consolidation: T7 == FD to 1e-6; T8 endpoint 1.53e-5 vs 3.41e-5; T10 bytes.
  - Close-out: demo; velocity recalibration (first two timeboxes of data).
PRL items:
  - REF-12 [M, 1.5 d] Circuit adjoint (5.6)-(5.8) on the UNFUSED gate list; sign_k = -1 for a transposed Ry; scatter-add. Accept: T7: adjoint == central FD (h = 1e-6) to 1e-6 on the 8 parameters, dLoss(y=0)/dtheta = [-0.1204452, ..., 0.9490759]; Sim15 effect-word sign check
  - REF-13 [M, 1.0 d] Tensordot reverse rule for the six shapes (direct-factor reverse rule, tree form, sign_k = +1). Accept: T24(a): 8 gradients == Sec. 8 dLoss to 1e-7 (4.8e-11 in fp64)
  - ERR-01 [M, 0.5 d] fp16 accumulation ban with the E-7 / E-8 stall tests. Accept: T8 (E-7 form): fp16 sum(2048) == sum(2049) == sum(4096) == 1.0; 2^-12 series stalls at 0.5; fp32 path to 1e-6
  - ERR-02 [M, 1.0 d] Q1.15 tensordot emulation: pre-scale S_q15 = 1 - 2^-15, store sat_int16((acc + 2^14) >> 15); gate path >> 14. Accept: T8: endpoint narrowing 1.53e-5 <= 2^-15 with pre-scale (3.41e-5 without); worked example abs(delta p0) 3.11e-5 / 3.47e-5 vs 2e-3; sweep max norm(delta sigma) 1.087e-4 <= 1.96e-4
  - EXP-01 [M, 1.0 d] Export writer and loader, format v1 (32 B header, 20 B records, uint16 half-angle turns). Accept: T10: magic, stride 20, type_sig n^r s n^l == 08 01 04 FF FF FF, decode(4172) = 0.399985 +- 1e-6, 0.8 + 2 pi -> different k; E-5 offsets == param_offset; V = 1000 Sim14 file 32,032 B
  - REF-15 [M, 1.0 d] Host trainer: Adam (5.9) at 16 B/param, unwrapped fp32 theta, clip 1, uniform [0, 2 pi) init with fixed seed, >= 3 restarts. Accept: E-10 tests: d2P_0/dt0 dt6 at theta = 0 == -0.25; E[D] at uniform init == 0.2500 at abs(K) = 2; dN_0/dtheta == 0 for Bob t2 and loves t1
  - REF-14 [S, 1.0 d] Gradient cross-checks: two-term + quotient and four-term shift rules, 4 pi periodicity, and the E-2 SPSA variance test. Accept: T7: Alice t0 = 0.0701761; loves t0 = -0.2073400; two-term on CRz (-0.0684908) asserted wrong; p_0(t + 4 pi) = 0.58263882 != p_0(t + 2 pi) = 0.87639280; E-2: variance 0.466300 flat in c at P = 8
  - ERR-04 [S, 1.0 d] E-11 readout policy: certificate 2e/(2 sqrt D - e), single D floor 2^-6 on both paths, fp32 fallback flag. Accept: T8 P_c part: P_c error <= 2e/(2 sqrt D - e); fp16-store bound at D = 2^-6 is 0.0197 (passes 0.05); D < 2^-6 routed to fp32
  - ERR-06 [C, 1.5 d] Host angle codec (octant reduction, degree-7/8 Taylor, optional 257-entry Q2.14 table of 514 B) and the E-9 65536-code sweep emulated on the host. Accept: sin <= 3.2e-7, cos <= 4e-8, degree 5/6 asserted to fail 2^-15; sweep 3.074e-5 vs 3.052e-5 + 3.1e-7 (tolerance open, G9)
  - REF-17 [C, 0.5 d] Real-only model path (re[] plane only, flags.bit2 = 1) for Sim15 / Ry-only words. Accept: Sim15 word states == complex path with zero imaginary part to 1e-12; memory halved
Demo: gradient table of the 8 Sec. 8 parameters (adjoint, FD, shift) side by side; Q1.15 worked example error
3.11e-5 vs the 2e-3 tolerance; an exported file round-tripped with T10 checks.
Review-record specific fields: T7 max abs(adjoint - FD); T8 endpoint and sweep values; export file sizes vs
32 + 20 V + 2 P_total; measured velocity (ideal days per timebox) vs the 10-day assumption.

#### TB3 -- P0 data, host training and early read (Weeks 7-8) [I1]

Objective: generate the P0 corpus and splits, train Model A on the host, train the direct-factor ablation, and
take gate G1.
  - Kick-off: Business Advisor (vacant) review of the split design requested; bridge procedure 8.5.
  - Investigation: sentence count check (1,190,592; review l550; scope/exp0.c); label-function property check;
    leakage checks on the splits (companion 13.2 c700-709 principles, applied to triples).
  - Refinement: greedy parser + program compile (six shapes); host training with uniform init and >= 3 restarts
    (errata E-10); direct-factor model at equal bytes.
  - Consolidation: accuracy per split for seed 1; B1 = 50 % sanity check if B1 done.
  - Close-out: G1 decision (Sec. 5.4); Deployment of I1 (release r0.1).
PRL items:
  - ERR-03 [M, 0.5 d] T-EZ Monte Carlo on the reference. Accept: T-EZ: mean Z over 1e5 Haar triples 0.250 +- 0.002 at (1,1), 0.0625 +- 0.0005 at (2,1)
  - P0-01 [M, 1.5 d] P0 CFG generator (S -> NP VP; NP -> N or ADJ N; VP -> TV NP or IV or IV ADV), 64-word lexicon (24 N, 12 ADJ, 12 TV, 8 IV, 8 ADV), and the label function (label(A TV B) != label(B TV A) on every role-swap pair; K = 2 for Model A, K = 8 for Model B). Accept: Distinct-sentence count == 1,190,592 (scope/exp0.c); all six shapes present; exhaustive role-swap check has 0 violations
  - P0-03 [M, 0.5 d] Split manifests and seeds: 4,000 train / 1,000 IID / 500 role-swap / 300 held-out triples / 200 paraphrase slots; 5 seeds. Accept: Manifests hash-pinned; no held-out triple in train; each role-swap pair seen only in the other order
  - P0-04 [M, 1.0 d] Greedy shift-reduce parser and program compile for the six shapes (host), with parse-FAIL status. Accept: T5/T6 parse steps; greedy exact on every generated P0 sentence (review l550)
  - P0-05 [M, 1.0 d] Model A: (1,1), IQP ansatz 0, L = 2, host training, seed 1. Accept: 176 angles = 352 B; peak 16 amplitudes (10 in place) = 128 B fp32; accuracy per split reported
  - P0-06 [M, 1.0 d] Direct-factor model at equal bytes (spec 1.6 vector model) as the ablation of the quantum parameterisation. Accept: Trained on the same split; accuracy per split beside Model A
  - P0-07 [M, 0.5 d] Gate G1 early-read decision record. Accept: Record: seed-1 IID >= 90 % and role-swap >= 90 %, or a redirect decision
  - P0-08 [S, 0.5 d] B1 bag-of-embeddings baseline (528 B, int8, own C++ autograd). Accept: B1 role-swap == 50 % by construction (review l555)
  - P0-09 [S, 1.0 d] Model B: (1,3), host training, seed 1. Accept: 320 angles = 640 B; 52 amplitudes = 416 / 208 B; K = 8
  - ERR-07 [S, 0.5 d] E-12 regression, greedy half: greedy must FAIL (never mis-parse) on both counterexamples. Accept: Greedy returns FAIL on "Alice loves man in park" and "Alice loves the man in park"
  - P0-10 [C, 1.0 d] Init diagnostics and op counter: D histogram and clip rate at uniform init; counted ops per sentence. Accept: Sec. 8 shape at uniform init: ~1.1 % of sentences with D < 2^-6, clip rate 52-54 %; op counter equals the hand count for the Sec. 8 sentence
  - TST-04 [C, 1.0 d] Small distributable fixture set with expected outputs. Accept: Fixture file loads in the harness; expected outputs from fp64
Demo: P0 sentence count and split manifests; Model A seed-1 accuracies on IID / role-swap / triple-holdout
beside the direct-factor model.
Review-record specific fields: G1 outcome and the measured values; D histogram and clip rate at init (if P0-10
done) against errata E-10 (1.1 % below 2^-6; 52-54 % clipped); manifest hashes.
GATE G1 at close-out.

#### TB4 -- CUDA forward path (Weeks 9-10) [I2]

Objective: register-resident Layout A fused tensordot and Layout B bent-wire forward at (1,1) and (1,3), equal
to the host reference.
  - Kick-off: Technical Advisor (vacant) review of the ptxas plan requested; bridge procedures 8.1, 8.7.
  - Investigation: instantiation list per (shape, q_n, q_s, ansatz, L) (spec 3.1 l530-531); register estimate
    (1,1) ~52 registers (spec l536-540).
  - Refinement: compile-time indexing (runtime indices demote arrays to local memory, spec 3.1 l528-533);
    bucketing by (shape, W); B3 baseline in parallel by an agent.
  - Consolidation: ptxas -v archived; device == host to 1e-6 on T4, T6(b), T6(c) and the P0 IID set.
  - Close-out: demo; carry-forward decisions.
PRL items:
  - CUD-01 [M, 2.5 d] CUDA Layout A thread-per-sentence fused tensordot, fp32, (1,1) and (1,3), with device word states from uint16 k via sincospif(k/32768.0f); one instantiation per (shape, q_n, q_s, ansatz, L); compile-time indices. Accept: T4 on device to 1e-6; T6(b) on device == fp64 to 1e-6; P0 IID outputs == host to 1e-6
  - CUD-02 [M, 0.5 d] Register-residency evidence: ptxas -v shows 0 B stack/local and 0 spills at (1,1) (~52 registers, 1024 threads/block) and (1,3). Accept: ptxas report archived; (1,1) launches at 1024 threads/block
  - CUD-04 [M, 1.0 d] Bucketing by (shape, W), angle gather from the table, C++ batch driver. Accept: Mixed-shape batch of the P0 IID set == host to 1e-6
  - CUD-05 [M, 1.5 d] Layout B warp-per-sentence bent-wire forward, W <= 6. Accept: T6(c) on device: psi[0], psi[2] == sigma to 1e-6, D = Z
  - CUD-06 [S, 0.5 d] Device readout with D, E-11 certificate and D-floor flag. Accept: Device P_c and D == host to 1e-6; flags identical
  - P0-13 [S, 1.5 d] B3 GRU d = 64 (28.6 KB) and the d = 8 GRU small-state control. Accept: Both trained on the P0 splits; accuracy per split reported
  - CUD-07 [C, 2.0 d] Layout C block-per-sentence (W <= 11) and register-blocked Layout B (W = 7..10). Accept: T6(c) in Layout C; (2,1) bent-wire N TV N (32 amplitudes) == fp64 to 1e-6
Demo: ptxas -v output (0 spill stores, 0 B local memory) and a P0 batch evaluated on the GPU matching the host.
Review-record specific fields: registers per instantiation; threads/block used; max abs(device - host); bridge
Sec. 10 E items settled (dated note).

#### TB5 -- CUDA training, export, B2 baseline (Weeks 11-12) [I2]

Objective: train P0 on CUDA with adjoint gradients and Adam; export the trained model; build B2.
  - Kick-off: bridge procedure 8.5; review decision "CUDA Layout A fp32 train" (review l569).
  - Investigation: tester session encodes device T24(a) and the step-equality test (CUD-10).
  - Refinement: device reverse rule with scatter-add; Adam kernel; logs of D histogram and clip rate per epoch
    (errata E-10 l145-146); export; B2 transformer with own C++ autograd.
  - Consolidation: training equality with the host; exported Model A file passes T10.
  - Close-out: G2; Deployment of I2 (release r0.2).
PRL items:
  - CUD-09 [M, 2.0 d] Layout A training: tensordot reverse rule on device (per-word recurrence, scatter-add into the flat gradient table). Accept: Device gradients == host REF-13 to 1e-6; T24(a) on device
  - CUD-10 [M, 1.5 d] Adam kernel over P_total (5.9), clip 1, eps_D = 1e-7, D histogram and clip-rate logs per epoch; export of the trained model (v1) with round trip. Accept: First 10 steps == host trainer to 1e-6 per parameter (same seed); T10 on the Model A file; size == 32 + 20 V + 2 P_total B
  - P0-12 [M, 2.0 d] B2 tiny transformer (d = 48, L = 2, 46.2 KB), int8, own C++ autograd. Accept: Trained on the P0 splits; accuracy per split reported
  - CUD-12 [S, 2.0 d] Circuit adjoint on Layout B with the Sim15 sign check on device; ptxas evidence for all training kernels. Accept: T7 on device: adjoint == FD to 1e-6, unsigned value has the opposite sign; 0 spill loads/stores recorded
  - CUD-14 [C, 1.0 d] Quantisation-aware forward: inference rounding emulated, fp32 gradients. Accept: Q1.15-emulated forward in training; accuracy delta reported
  - CUD-15 [C, 1.0 d] GPU throughput per shape (sentences per second). Accept: Recorded with GPU model and driver version
Demo: a P0 Model A training run on the GPU with per-epoch D histogram; export file loaded back; B2 trained.
Review-record specific fields: G2 outcome; training wall time (recorded, not claimed); D histogram per epoch
(input to bridge Sec. 10 K); B2 bytes.
GATE G2 at close-out.

#### TB6 -- HVX bring-up and Q1.15 sentence-per-lane (Weeks 13-14) [I3]

Objective: bring up HVX through the adapter (T9) and deliver the (1,1) Q1.15 sentence-per-lane fused tensordot
with the E-9 corrected register budget, accurate against fp64.
  - Kick-off: Technical Advisor (vacant, Hexagon) consulted; bridge worked route 8.1a read in full.
  - Investigation: T9 on the device: ramp through vmpy/vasr, vdelta controls, fl(a*b) and subnormals (spec
    l658-663: lane-order mistakes silently permute amplitudes).
  - Refinement: lane-varying Q2.14 coefficient vectors (3 registers per rotation gate, errata E-9 l104-107);
    pre-scale S_q15; store with Rt8 = 15 (spec l918); D-floor fallback.
  - Consolidation: register report (peak <= 29 of 32, 0 spills); T6(b) within 2e-3; measurement note.
  - Close-out: demo; bridge Sec. 10 B items settled moved to a dated note.
PRL items:
  - QC-01 [M, 0.5 d] Adapter boundary contract: model image (angle table + offset lookup), batch submit, outputs (2^{q_s} + 1 values, D, status), VTCM reservation. Accept: Adapter conformance test: load, submit, read back on the DSP; resident model = angle table + 0 or 4 B per word
  - QC-02 [M, 1.0 d] T9 HVX bring-up through the adapter. Accept: T9: vmpy/vasr ramp round trip is the identity; vdelta xor controls k = 0..5 match a lane-id vector; fl(a*b) rounding and subnormals recorded
  - QC-03 [M, 0.5 d] E-9 extension on the device: vector-form coefficient ramp and 65536-code sincos sweep. Accept: Sweep error compared with 3.074e-5 (host); tolerance fixed by a proposed erratum (G9)
  - QC-04 [M, 2.0 d] HVX Q1.15 sentence-per-lane (1,1) fused tensordot: lane-varying Q2.14 coefficient vectors, S_q15 pre-scale, Rt8 = 15 store, int32 accumulate. Accept: Register peak <= 29 of 32 (tensordot 24, CRz 27, Rx 28-29), 0 spills; WS = 10 <= 11
  - QC-05 [M, 0.5 d] On-device accuracy: T6(b) in Q1.15 and the device parts of T8. Accept: T6(b) within 2e-3 of fp64; worked-example error of the order of 3.11e-5; no int32 beyond 2^30
  - QC-06 [M, 0.5 d] D-floor rejection (D < 2^-6) with fp32 fallback and certificate on the Q1.15 path. Accept: Every sentence with D < 2^-6 re-run in fp32 and flagged; certificate emitted
  - QC-07 [S, 1.5 d] HVX data-path options: exported Q15 (cos, sin) pairs (flags.bit0 = 1, 0 DV) or Q1.31 polynomial via vmpye/vmpyo (~36 DV per 64 angles, estimate); sorted-batch fallback where vgather (v65+) is absent. Accept: Option recorded; T10 with flags.bit0 = 1; device T4 within 2e-3; sorted-batch output == vgather output bit for bit
  - QC-08 [S, 0.5 d] Register/spill report and measured cycles as a C14 target measurement; settled bridge Sec. 10 B items moved to a dated note. Accept: Dated measurement note naming part and toolchain
  - QC-10 [C, 1.0 d] fp16-store path on v68+ with hvx-ieee-fp (25-29 registers). Accept: T6 fp16-store within 3e-3; FTZ behaviour recorded
  - QC-11 [C, 1.0 d] HMX / Cloud AI 100 probe: one 32x32 fp16 tile against fp64 (only if the adapter exposes HMX). Accept: Tile error recorded; never used for probability sums
Demo: T9 ramp round trip on the device; the Sec. 8 sentence evaluated in Q1.15 on HVX with abs(delta p0)
against 3.11e-5 (errata l87) and the 2e-3 tolerance.
Review-record specific fields: HVX part, ISA version, toolchain, adapter version; measured register peak per
kernel phase (tensordot, CRz, Rx butterfly); coefficient-generation cycles if measured (vs the ~36 DV estimate,
errata l112); fl(a*b) rounding mode and subnormal behaviour; vgather available yes/no.

#### TB7 -- (1,3) on HVX, P0 sets on device, Adreno (Weeks 15-16) [I3]

Objective: run both P0 models' test sets end to end through the adapter against the fp32 reference, measure the
working set, and (Should) bring up Adreno.
  - Kick-off: decide the (1,3) HVX layout (gap G31, Appendix A): amplitude-per-lane or VTCM-streamed; record the
    decision as a proposal to the addendum owner.
  - Investigation: layout spike (1 day) on register count vs VTCM streaming (spec 3.2 l666-675: no "W = 10 cliff").
  - Refinement: (1,3) kernel; end-to-end harness; certificates and fallback accounting.
  - Consolidation: working-set measurement (QC-14) for F3; accuracy report.
  - Close-out: G3; Deployment of I3 (release r0.3).
PRL items:
  - QC-12 [M, 1.5 d] Model B (1,3) Q1.15 inference on HVX: layout selection (gap G31) and kernel. Accept: 52-amplitude working set (208 B Q1.15) measured; outputs within 2e-3 of fp64
  - QC-13 [M, 2.0 d] Full P0 test sets through the adapter end to end (word ids in; 2^{q_s} + 1 outputs, D and status out) against the fp32 reference. Accept: Classification agreement reported; max abs(delta P_c) <= certificate per sentence; fallback rate reported; traffic tens of bytes in and 12 B out at (1,1) (spec l1166)
  - QC-14 [M, 0.5 d] Measured state bytes and counted ops per sentence on the DSP. Accept: Working set <= 128 B (Model A) and <= 416 B (Model B) (falsifier F3, review l563)
  - QC-16 [S, 2.0 d] Adreno fiber-per-sentence fused tensordot (1,1), fp16-store / fp32-compute, through the adapter, with compiler register/spill report. Accept: T6 in fp16 storage within 3e-3; 0 spills (spills go to system memory)
  - QC-18 [C, 1.5 d] HVX amplitude-per-lane (2,1) kernel with 5 vror + vadd reductions. Accept: (2,1) N TV N within 2e-3 of fp64
Demo: the P0 IID set classified on the DSP with agreement vs fp32 and the fallback rate; measured working set
against 128 B / 416 B.
Review-record specific fields: G3 outcome; G31 decision; measured state bytes per model; fallback rate at the
D floor; Adreno compiler report (if done).
GATE G3 at close-out.

#### TB8 -- P0 formal runs and falsification decision (Weeks 17-18) [I4]

Objective: execute P0 exactly as predeclared (review Sec. 7) and take gate G4.
  - Kick-off: freeze manifests, seeds and code (hash-record them); Business Ambassador (vacant; recommended
    recruit) authors the 200 paraphrases; Business Advisor (vacant) reviews the metrics plan.
  - Investigation: dry run of the metrics table on one seed.
  - Refinement: 5 seeds x (Model A, Model B, B1, B2, B3, d = 8 GRU, direct-factor); parse-FAIL on paraphrases;
    device evaluation of the QNLP models.
  - Consolidation: metrics table complete; uncertainty per threshold (Should).
  - Close-out: G4 workshop (Sec. 11.1) and decision record.
PRL items:
  - P0-14 [M, 1.0 d] 200 hand-written paraphrases authored by someone other than the engineer. Accept: 200 items with author and date; parse-FAIL rate computed
  - P0-15 [M, 1.0 d] 5-seed training of Models A and B on CUDA, >= 3 restarts per seed. Accept: Accuracy per split per seed; D histograms at init and after training
  - P0-16 [M, 1.0 d] 5-seed training of B1, B2, B3, the d = 8 GRU and the direct-factor model; int8 export. Accept: Accuracy per split per seed; bytes per model (B1 528 B, B2 46.2 KB, B3 28.6 KB)
  - P0-17 [M, 1.0 d] P0 metrics table: accuracy on IID / role-swap / triple-holdout, parse-FAIL rate, state bytes, ops, registers/spills, D histogram and clip rate. Accept: Every metric populated for every model and seed
  - P0-18 [M, 0.5 d] Gate G4 falsification decision record. Accept: Decision per Sec. 5.4 of this plan, signed by the Business Sponsor (or the interim)
  - P0-19 [S, 0.5 d] Uncertainty across seeds for every predeclared threshold. Accept: Mean, sd and interval reported beside every threshold
  - P0-20 [S, 1.5 d] Ablations: real vs complex (Sim15 / ansatz 4, flags.bit2) and exported Q1.15 model including fallback vs the fp32 reference. Accept: Accuracy at equal bytes; accuracy delta and fallback rate reported
  - P0-22 [C, 2.0 d] Further benchmarks: MC (130) of Lorenz et al. on the six shapes; closed-domain intent set (8-32 intents) at (1,3) / (1,5). Accept: Accuracy, parse-FAIL, bytes and ops reported; literature figures stay UNVERIFIED (bridge Sec. 10 J)
Demo: the P0 metrics table against the predeclared thresholds and the four falsifiers, live from the evidence
logs.
Review-record specific fields: G4 outcome (PASS / PASS-WEAK / FALSIFIED by F1-F4); per-seed accuracies; bytes
per model; parse-FAIL rate; D histograms at init and after training; who signed.
GATE G4 at close-out (decision); record completed in TB9.

#### TB9 -- P0 evidence, consolidation and Deployment (Weeks 19-20) [I4]

Objective: deploy the P0 increment with a reproducible evidence package, whatever the verdict.
  - Kick-off: Deployment Plan for I4 confirmed (Sec. 3.5).
  - Investigation: clean-machine reproduction attempt.
  - Refinement: evidence package; one-command reproduction; negative-result record; document maintenance
    (bridge 8.7 step 4; errata proposal for the T9 tolerance, G9).
  - Consolidation: bridge re-issued if Sec. 10 items were settled; manifest validates with jq.
  - Close-out: Project Review Report (I4); interim Benefits Assessment input; if G4 = STOP, project closure
    starts here.
PRL items:
  - DEP-01 [M, 1.5 d] Evidence package: frozen grammar and lexicon, manifests, seeds, configs, fixtures with expected outputs, compiler versions, spill reports. Accept: Package reproduces every P0 number from a clean checkout
  - DEP-02 [M, 1.0 d] One documented command for reference checks and a separate command for the P0 demonstration. Accept: Both commands run on a clean machine with meaningful exit codes
  - DEP-03 [M, 0.5 d] Negative-result record. Accept: Every failed threshold and unsupported case listed with its cause class
  - DEP-04 [M, 1.0 d] Document maintenance after P0: dated measurement notes for settled UNVERIFIED items; erratum proposal for the T9 tolerance (G9). Accept: Bridge re-issued; manifest validates with jq
  - DEP-05 [M, 0.5 d] Increment deployment: Project Review Report (increment) and Benefits Enablement. Accept: Report filed; measured values of benefits BN-1..BN-9 recorded
  - DEP-06 [S, 1.0 d] Retabulate spec Sec. 4.6 right-hand columns under E-11 (G28) with a C program; erratum drafted. Accept: Two independent programs agree
  - DEP-07 [S, 1.0 d] Output status record struct (G20): structural validity, task score, numerical validity with certificate, approximation status, fallback used. Accept: Struct in the adapter header; every output carries it
  - DEP-08 [C, 1.0 d] Export-field proposals as a C15 version bump: ring order and closing H (G2, G16); r, chi, readout, grammar version, checksum (G19). Accept: Proposals logged in bridge Sec. 12
  - DEP-10 [C, 1.0 d] Tractability-contract record for the P0 profile (G18). Accept: Seven fields populated with measured values
Demo: reproduction of the P0 table from a clean checkout with the two documented commands.
Review-record specific fields: reproduction time and machine; list of UNVERIFIED items closed with dates; errata
proposals filed; decision on continuing to I5 (from G4).

#### TB10 -- Chart parser, lexicon, build_tree (Weeks 21-22) [I5]

Objective: replace the greedy parser by the chart recogniser as the production parser (errata E-12) and build
word trees for arbitrary cup structures.
  - Kick-off: bridge procedure 8.2 (add a grammar construction) and its relative-clause line map.
  - Investigation: tester session encodes T11 rows (addendum l1826) and the E-12 counts.
  - Refinement: chart with saturating counts; lexicon rows; build_tree with cycle wrappers and CHAIN nodes.
  - Consolidation: T11 on 1e5 random strings vs exhaustive search (0 false accepts / rejects).
  - Close-out: demo.
PRL items:
  - GRM-01 [M, 2.0 d] Chart recogniser (11.4)-(11.5): saturating counts, first / k-th parse, accept, accept_prefix; host or DSP scalar. Accept: T11: every 11.2 row exact; "Alice sleeps and dreams" -> (2,3)(1,4)(0,5)(8,9)(7,10), s at 6; 0 false accepts/rejects on 1e5 strings
  - GRM-02 [M, 0.5 d] E-12 full regression (chart half). Accept: Unique parse (0,1)(4,5)(3,6)(7,8), s at 2 for both counterexamples; count == (m-1)(m+1)(m+3)/24 for m = 5..197
  - GRM-03 [M, 1.0 d] Sec. 11.2 lexicon over {N, S, I, Q} (C15); subject relative n^r n s^l n, object relative n^r n n^ll s^l. Accept: Every row returns its listed cups and survivor via GRM-01
  - GRM-04 [M, 1.5 d] build_tree: word graph, spanning tree, cycle wrapper, CHAIN nodes, post-order by decreasing peak (11.18'); 14 B Node; nchild <= 4. Accept: Tree for the T12 sentence matches the addendum; asserts fire on nchild > 4
  - GRM-05 [S, 1.5 d] Ambiguity accumulators (11.25) / (11.26) and host chart memory measurement. Accept: Two-parse string: kth(0) != kth(1) (T11); accumulators 16 B / 32 B; chart 1.375 m^2 B: m = 92 -> 11,638 B, m = 256 -> 90,112 B
  - GRM-07 [S, 0.5 d] Greedy kept as a six-shape pre-check only, with dispatch to the chart. Accept: Greedy used only when every word is in the six-shape table
  - GRM-08 [C, 2.0 d] Rule-based POS tagger front end (size and accuracy UNVERIFIED). Accept: Tagger bytes and accuracy on a public set measured
Demo: "Alice loves man in park": greedy FAIL, chart unique parse (0,1)(4,5)(3,6)(7,8), s at 2; T12 word tree
printed.
Review-record specific fields: chart partner-test counts vs (m-1)(m+1)(m+3)/24; host memory for m = 92 and 256.

#### TB11 -- Node ops, spiders, tree adjoint (Weeks 23-24) [I5]

Objective: evaluate and differentiate any Sec. 11 sentence on the host (relative clauses, coordination).
  - Kick-off: addendum 11.3-11.4, 11.7; bridge 8.2 step 2 contraction order.
  - Investigation: T12-T14 and T24 vectors encoded from addendum/t18.c and critic/adjtree.c.
  - Refinement: node ops per (type string, cmask, pmask); spider loop at 0 B (who / whom by index equality);
    tree adjoint with scatter-add.
  - Consolidation: T22 memory program written (none on disk).
  - Close-out: demo.
PRL items:
  - GRM-09 [M, 2.5 d] Node ops: tensordot per (type string, cmask, pmask), spider loop, chain matmul, Delta. Accept: T12: Z = 0.13192995, p = [0.33023573, 0.66976427], Loss(y=0) = 1.10794854; whom Z = 0.15647590; T13: "Alice and Bob sleep" Z = 0.04209530; T14 to 1e-15
  - GRM-10 [M, 2.0 d] Tree adjoint for arbitrary trees: sign_k = +1, spider and chain adjoints, M_train accounting (11.31). Accept: T24(b)-(e): T12 sentence gradients as listed, M_train = 50; (d) L = 0.68668626; omitting scatter-add FAILs
  - GRM-11 [M, 0.5 d] T22 scheduler memory program (none on disk). Accept: Two relative clauses 16 / 48 (12 / 40 in place); believes sentence 14 / 44; streaming 20 / 80
  - GRM-12 [S, 1.0 d] (2,1) relative-clause fp64 vector (none exists): extend addendum/t18.c, append to addendum Sec. 18. Accept: Brute force over the product state agrees to 1e-12; addendum re-hashed
  - GRM-13 [S, 1.0 d] CUDA node-op kernels for the relative clause at (1,1) and (2,1). Accept: 0 spills; peak 14 (10) / 44 (36) amplitudes
  - GRM-14 [C, 1.5 d] HVX Q1.15 node op for the (1,1) relative clause. Accept: Within 2e-3 of fp64; register report
  - GRM-15 [C, 0.5 d] Nested cup-pairing test vector at q >= 2 (flags.bit1 = 1). Accept: fp64 vector produced by two programs
Demo: "Alice who loves Bob sleeps" Z = 0.13192995 and its 9 gradients; "Alice and Bob sleep" Z = 0.04209530.
Review-record specific fields: peak amplitudes per sentence vs (11.18') (counting convention per bridge
conflict row 36: 14 / 44 vs 16 / 48 stated); M_train measured.

#### TB12 -- Export v+1, colouring, grammar training and evaluation (Weeks 25-26) [I5]

Objective: ship the grammar increment: export v+1, circuit colouring, CUDA training of Sec. 11 sentences, and
the coverage measurement on the P0 paraphrases.
  - Kick-off: bridge procedure 8.4 (export) with C15.
  - Investigation: T18 colourings from addendum/t18b.c.
  - Refinement: export v+1; colouring; device tree adjoint; parse-FAIL on paraphrases.
  - Consolidation: RP benchmark (Should) if data available.
  - Close-out: G5; Deployment of I5 (release r1.1).
PRL items:
  - GRM-16 [M, 1.0 d] Export v+1 (C15): base codes 2 = i, 3 = q; header q_i, q_q; spider marker 0xFE; version + 1. Accept: T10 extended: round trip of a relative-clause model; version incremented
  - GRM-17 [M, 1.0 d] Circuit-form colouring (11.22): transparent spiders, Delta ancillas, Bell fallback. Accept: T18: "Alice who loves Bob sleeps" states {loves, sleeps}, effects {Alice, Bob}, W_hw = 3; "Alice and Bob sleep" W_hw = 3; triangle has one Bell cup
  - GRM-18 [M, 1.5 d] CUDA training of Sec. 11 sentences via the tree adjoint. Accept: Device gradients == host T24(b) to 1e-6; M_train 400 B for the T12 sentence
  - GRM-19 [M, 0.5 d] Parse-FAIL rate of the 200 P0 paraphrases with the Sec. 11 lexicon. Accept: Rate reported beside the expected 10-30 % (a hypothesis)
  - GRM-20 [S, 2.0 d] Grammar evaluation: RP (105) relative-pronoun benchmark (readout on the surviving n wire) and the grammar ablation. Accept: Accuracy reported with dataset provenance; coverage and accuracy against construction count
  - GRM-22 [C, 2.0 d] Proposals: canonical Sec. 11 circuit qubit numbering (G13), tree parameter order (G12), binary encoding of cups[] / tree[] (G4). Accept: Proposals logged; cups[] / tree[] round-trip test passes
Demo: a relative-clause model exported (v+1), reloaded, and trained on the GPU; the paraphrase parse-FAIL rate
with the Sec. 11 lexicon.
Review-record specific fields: G5 outcome; parse-FAIL rate vs the 10-30 % expectation (addendum 15.4 item 5,
a hypothesis); export version value used.
GATE G5 at close-out.

#### TB13 -- MPO verbs, mixed radix, (4,2) precision (Weeks 27-28) [I6]

Objective: low-rank verbs and the precision study at (4,2).
  - Kick-off: addendum 12.3, 12.5; bridge procedure 8.3.
  - Investigation: T16 / T17 values encoded (no programs on disk: bridge G10).
  - Refinement: two-stage contraction; isometry generation; norm projection; T23 Monte Carlo.
  - Consolidation: E-11 retabulation (Should, G28).
  - Close-out: demo.
PRL items:
  - CAP-01 [M, 2.0 d] MPO verb two-stage contraction (12.14) with streamed cores, and the mixed-radix odometer (12.16'). Accept: T17: V = [21, 0, 36, 3, 1, -4, 6, -3], u = (7,0), w = (6,2), sigma = (70, 126) dense and streamed, B read once; T16: (3,3,3): (2,1,0) -> 5, (1,2,2) -> 25, 17 -> (2,2,1); (3,2,3): 13 -> (1,0,2); odometer 0..17 in order
  - CAP-03 [M, 1.5 d] Isometry generation via U_A^T / U_C (12.15) and operator-norm projection (12.16) by power iteration. Accept: Operator norm <= 1 after projection (to 1e-6 against an fp64 SVD)
  - CAP-04 [M, 1.0 d] (4,2) precision study. Accept: T23: mean Z 0.25 / 0.0625 / 0.0039 +- 10 % over 400 triples; Q1.15 (4,2) vs fp32 <= 2.5e-2 in p (loose ceiling)
  - CAP-05 [S, 1.0 d] Retabulate addendum a708-709, the T23 threshold and the 14.2 clause table under E-11 (G28). Accept: Two programs agree on 1.25e-2 and p_min 1.05e-4 or correct them; erratum drafted
  - CAP-06 [S, 1.0 d] Training-dependent Z: D histograms per epoch in a (4,2) run. Accept: Histograms logged; finding on whether training raises Z above 2^{-2 q_n}
  - CAP-07 [C, 2.0 d] Ansatz capacity items: Jacobian ranks at Q = 4, 5 (sufficiency of L_sat) and a defined ansatz_id 5 with a test vector (G3). Accept: Ranks recorded against dof 2^{Q+1} - 2; ansatz 5 gate list, P_w, transpose rule and fp64 vector
Demo: T17 sigma = (70, 126) dense and streamed; T23 mean Z at (4,2) against 0.0039 +- 10 %.
Review-record specific fields: measured Q1.15 (4,2) error vs the 2.5e-2 ceiling and the UNVERIFIED 1.25e-2;
D histogram in a (4,2) run (bridge Sec. 10 K).

#### TB14 -- Hashed n-gram angle generator and its export (Weeks 29-30) [I6]

Objective: generate angles from features (open vocabulary) with the zero-table front end, export the generator
block, and train it.
  - Kick-off: addendum 13.1, 13.4, 13.7.
  - Investigation: T19 vectors from addendum/mix.c.
  - Refinement: generator with safe cast and integer path; non-zero bias (errata E-10 l147); segmented-reduction
    gradients; export block with loader checks.
  - Consolidation: hybrid evaluation (Should).
  - Close-out: G6; Deployment of I6 (release r1.2).
PRL items:
  - CAP-09 [M, 2.0 d] Angle generator (13.1)-(13.2): safe cast, integer path, FNV-1a, mix32 zero table, integer mean, type classifier with FAIL-retry. Accept: T19: -1.0 -> 60321; 0.8 -> 4172; 13.0 -> 2261; NaN -> 0; acc = 1000 -> 4172 +- 1; mix32(0) = 0x92CA2F0E; FNV-1a("<the>") = 0xFCF0F1C0
  - CAP-10 [M, 1.0 d] Generator export block (13.7) with flags.bit3, class records and loader checks. Accept: Loader rejects a Cc mismatch; T19 vectors pass; (1,1) IQP L = 2, m = 32 block = 808 B
  - CAP-11 [M, 0.5 d] Non-zero generator bias at init. Accept: Zero-bias init shown to sit on the theta = 0 saddle (mixed derivative -0.25); non-zero bias leaves it
  - CAP-12 [M, 1.0 d] Hybrid gradients by segmented-reduction scatter. Accept: Generator gradients == FD to 1e-6 on a (1,1) example
  - CAP-13 [S, 2.0 d] Hybrid evaluation (head over hashed features vs a linear probe) and hybrid memory rows measured. Accept: Accuracy and bytes for both; zero-table front end 0 B + 429 B at (1,1), m = 32
  - CAP-15 [C, 1.5 d] Context gate (13.9) with BPTT. Accept: 8 KB / 16 KB int8 measured; gradient == FD
  - CAP-16 [C, 0.5 d] On-device per-token generator pipeline (13.5). Accept: Device k == host k for 1e4 tokens
Demo: T19 codec and hash vectors; a generator block of 808 B loaded with the Cc / mix32(0) checks.
Review-record specific fields: G6 outcome; generator bytes vs addendum l1150; front-end bytes vs Sec. 16 rows.
GATE G6 at close-out.

#### TB15 -- DisCoCirc and spectral readouts (Weeks 31-32) [I7]

Objective: texts as gates on persistent noun wires with RDM, entropy, purity and mutual-information readouts.
  - Kick-off: bridge procedure 8.9 (RDM / spectral readout); addendum C17.
  - Investigation: T20 written (no program exists); T-W3 / T-W4 / T-SF values from limits l452-467, l740-777.
  - Refinement: build_gates with argument order and pi permutation; RDM in fp32; Jacobi with the fp32 clamp
    1e-7 (bridge conflict row 41).
  - Consolidation: dense vs gate-by-gate equality (T-W4).
  - Close-out: demo.
PRL items:
  - DSC-01 [M, 2.0 d] DisCoCirc build_gates: C17 argument resolution, emission order, pi permutation when i_subj > i_obj, compile-time switch over field pairs. Accept: T20 (write it): wires (0,1) and (1,0) give identical rho_Alice to 1e-12; U_big before U_loves; real Sim15 verb FAILs, Sim14 passes
  - DSC-02 [M, 1.0 d] RDM readout (14.19): fp32 accumulation, ket row / bra column, division by Tr rho. Accept: 128 B accumulator at q_n = 2; limits T-W3 at q_n = 1 reproduced
  - DSC-03 [M, 1.5 d] Spectral functionals (limits A2): d = 2 closed form, d = 4 cyclic Jacobi with 3 sweeps, fp32 clamp lam <= 1e-7, log base stated. Accept: T-SF1..T-SF4: lam 0.93996159 / 0.06003841, S = 0.22707320 nat, purity 0.88713240, F_sq 0.93359620, T 0.09984988
  - DSC-04 [M, 1.5 d] All-wires partial trace in one pass (limits A4). Accept: T-W4: eig 0.97766824 / 0.02233176 / 0 / 0, S = 0.15433977 bit, purity 0.95633390, I = 0.30867954 bit; dense 16x16 path == gate-by-gate to 1e-6
  - DSC-05 [S, 1.0 d] coref_resolve interface (string-equality coreference). Accept: Interface and fixture; accuracy recorded as UNVERIFIED
  - DSC-06 [S, 1.0 d] Proposals: eps for fidelity readouts (G11) and one packed-Hermitian layout (G7). Accept: Proposals with a derivation program
  - DSC-07 [C, 2.0 d] Non-unitary word maps on density matrices (limits A1). Accept: A1-T1..T5b: fuzz diag(0.5, 0.125), T = 0.625; adjoint P = 0.8520079519
Demo: a two-sentence text with rho_Alice, S = 0.15433977 bit and I(Alice:Bob) = 0.30867954 bit (T-W4).
Review-record specific fields: log base stated per output; clamp used; working set per wire vs <= 400 B (limits
l418-421).

#### TB16 -- MPS text state (Weeks 33-34) [I7]

Objective: bounded-memory contraction of texts by an MPS over noun wires.
  - Kick-off: addendum 14.3; T15 / T21 programs (derivH/jsvd.c, mpstest.c; verH/).
  - Investigation: rank-guard and sort cases first (verH/mps_nan.c, jsort.c).
  - Refinement: two-site update, sorted one-sided Jacobi, QR centre moves, log Z carry; MPS RDMs.
  - Consolidation: 8 KB slice measurement (Should).
  - Close-out: demo.
PRL items:
  - DSC-08 [M, 3.0 d] MPS: two-site update, rank guard S_tol = 1e-6, sorted one-sided Jacobi incl. m < n, QR centre moves, log Z carry. Accept: T15: S = [3, 2.2360679775, 2.2360679775, 1], residuals <= 2e-15; rank guard S = [1,0,0,0] -> chi_i = 1, no NaN; T21: chi = 8, N = 6, 30 U(4) gates, error <= 1e-10
  - DSC-09 [M, 1.0 d] MPS single- and two-wire RDMs (14.19') and Uhlmann fidelity by two Hermitian Jacobi calls. Accept: Two-wire RDM from the MPS == exact RDM to 1e-6 in fp32 at exact chi
  - DSC-10 [S, 1.5 d] Gram matrix, InfoNCE and slot scoring (limits A3). Accept: A3-gram: Z_0 = 0.14026553, Z_1 = 0.16045040, F_01 = 0.97273229, InfoNCE L = 0.53396114
  - DSC-11 [S, 0.5 d] MPS memory in the 8 KB slice regime measured. Accept: N = 32, chi = 4: 7,488 B fp32 / 3,744 B fp16 + 1.0 KB fp32 scratch
  - DSC-12 [C, 1.5 d] DisCoCirc higher-order boxes (limits A5). Accept: A5 Tests A-H: p(Claire = 1) = 0.221700; hedge Z = 0.526433
  - DSC-13 [C, 0.5 d] Postselection norm as a plausibility score (limits Table B row B1). Accept: B-parse: parse 0 Z = 0.07915003, parse 1 Z = 0.05651013; q = [0.58344344, 0.41655656]
Demo: T21 MPS vs exact error <= 1e-10 at chi = 8, N = 6; discarded weight equal to the error in the truncated
case (1.379e-2).
Review-record specific fields: chi, discarded weight and memory per run; any NaN guard firing.

#### TB17 -- Length split, discourse on device, programme evidence (Weeks 35-36) [I7]

Objective: the productivity experiment on stories and the final evidence update.
  - Kick-off: addendum 15.4 item 2(iii) protocol (train <= 4 actors, test 8-16; 5 seeds).
  - Investigation: story generator and split manifests (reusing the P0 manifest tooling).
  - Refinement: training and evaluation; device text register (Should).
  - Consolidation: evidence package update; negative results recorded.
  - Close-out: G7; Deployment of I7 (release r1.3); inputs to the final Project Review Report.
PRL items:
  - DSC-14 [M, 2.0 d] Length / productivity split: train stories with <= 4 actors, test 8-16. Accept: Generalisation and in-distribution accuracy over 5 seeds (outcome is a hypothesis)
  - DSC-15 [M, 1.5 d] Programme evidence package updated with the I5-I7 numbers; discourse increment deployment record; Benefits Assessment inputs. Accept: Package reproduces every I5-I7 number from a clean checkout; record filed
  - DSC-17 [S, 1.5 d] DisCoCirc text register on device: N = 4 nouns, q_n = 1 (16 amplitudes, 128 / 64 B) on CUDA and HVX. Accept: Within 1e-6 (fp32) and 2e-3 (Q1.15) of fp64
  - DSC-18 [S, 0.5 d] Entropy gradient (limits A4-grad). Accept: dS_0/dtheta_CRz = 0.42460500
  - DSC-19 [C, 1.0 d] bAbI-derived QDisCoCirc QA set. Accept: Accuracy reported; dataset provenance recorded
  - DSC-20 [C, 1.0 d] Exact second-order training diagnostics (limits A6). Accept: A6-et: J[0][7] = 0; F_cl eigenvalue 1.687955; E[D] = 0.2500; DLA dim 13 of 63
Demo: generalisation vs in-distribution accuracy on 8-16-actor stories after training on <= 4.
Review-record specific fields: G7 outcome; the length-split result against the addendum 15.4 item 5 hypothesis.
GATE G7 at close-out.

#### Final Deployment (Weeks 37-38)

Not a development timebox. Activities: consolidated release; final Project Review Report (Sec. 3.5 template);
Benefits Assessment plan confirmed (Sec. 12.4); bridge and manifest re-issued with all measurement notes;
hand-over of open risks (Sec. 8) with owners.


## 6. Solution Architecture Definition (summary)

Technical authority for everything in this section is the cited document; this section only arranges it into
modules, interfaces and delivery order.

### 6.1 Module map

Bridge-procedure column = the bridge Sec. 8 procedure an agent follows. UNVERIFIED column = bridge Sec. 10 group
letters (and gaps) the module depends on.

| Module | Name | Source | Runs on | Bridge procedure | Depends on UNVERIFIED / gaps | Increment |
|---|---|---|---|---|---|---|
| M0 | Harness, CI, evidence tooling (ctest, sha256sum, jq) | spec Sec. 9 tests; companion 16.1; bridge G10 | host | -- | O (document hygiene); G10, G23 | I0-I1, all |
| M1 | Lexicon, greedy pre-check parser, PROGRAM compile | spec Sec. 9 item 1; spec 1.1, 2.6; addendum 11.2 | host | 8.2 | H (front-end statistics); G17 | I1, I5 |
| M2 | Angle codec (uint16 turns <-> cos/sin; Q15 pairs) | spec Sec. 9 item 2; spec C5, 4.4; errata E-9 | host, CUDA, HVX | 8.3 | B (coefficient cycles ~36 DV); F (T9 tolerance); G9 | I1, I3 |
| M3 | Word-state generator, ansatz ids 0-4 | spec Sec. 9 item 3; spec C10 | all | 8.1 | A (ring order, closing H); G (L, real-only); G2, G3 | I1, I2 |
| M4 | Butterflies and 2q gates | spec Sec. 9 item 4; spec 2.1-2.2 | host, CUDA | 8.1 | E (register counts) | I1 |
| M5 | Fused tensordot and matmul form | spec Sec. 9 item 5; spec 1.3, 2.5, 4.7; errata E-6, E-13 | all | 8.1, 8.1a | B, D, E (residency); F | I1-I3 |
| M6 | Postselect-compact, Bell cups, wire_map | spec Sec. 9 item 6; spec 1.4, 2.3 | host, CUDA | 8.1 | G5 (nested cups); G6 (wire_map names) | I1 |
| M7 | Readout (N_c, D, P_c, guards, certificate, fidelity) | spec Sec. 9 item 7; spec 1.5, 5.1; errata E-11 | all | 8.3 | F; G20 (status record) | I1-I3 |
| M8 | Adjoint (circuit form) and Adam | spec Sec. 9 item 8; spec 5.3, 5.5, 6.1; errata E-10 | host, CUDA | 8.5 | E; K (training-dependent Z) | I1-I2 |
| M9 | Parameter-shift and SPSA cross-checks | spec Sec. 9 item 9; spec 5.2, 5.4; errata E-2 | host | 8.5 | F | I1 |
| M10 | Export writer / loader v1 | spec Sec. 9 item 10; spec 6.3; errata E-3, E-5 | host, device loader | 8.4 | G2, G16, G19 | I1-I2 |
| M11 | Back ends: CUDA Layouts A/B/C; HVX sentence-per-lane and amplitude-per-lane; Adreno fiber-per-sentence | spec Sec. 9 item 11; spec 3.1-3.4; errata E-9, E-13 | CUDA, HVX, Adreno | 8.7, 8.1a | B, C, D, E; G24; G31 | I2-I3 |
| M12 | Chart recogniser | addendum Sec. 18 item 12; 11.1; errata E-12 | host / DSP scalar | 8.2 | H; G26 | I5 |
| M13 | build_tree | addendum item 13; 11.4; C16 | host | 8.2 | G4, G12; puremath App. B items 18-19 | I5 |
| M14 | Node ops (tensordot per node, spider loop, chain, Delta) | addendum item 14; 11.3-11.4 | all | 8.2 | E, B (residency) | I5 |
| M15 | Export v+1 (C15) and generator block | addendum item 15; C15; 13.7 | host, device loader | 8.4 | G19 | I5-I6 |
| M16 | Circuit-form colouring | addendum item 16; 11.5 | host | 8.2 | G13; puremath App. B item 8 (Conjecture 2.34) | I5 |
| M17 | MPO verb and mixed-radix odometer | addendum item 17; 12.5, (12.16') | all | 8.1 | G | I6 |
| M18 | Angle generator (hashed n-gram, zero table) | addendum item 18; 13.1-13.5 | host, CUDA, HVX | 8.1 | I (hybrid front end) | I6 |
| M19 | DisCoCirc text register | addendum item 19; C17; 14.1, 14.5 | host, CUDA, HVX | 8.9 | H (coreference); M | I7 |
| M20 | MPS | addendum item 20; 14.3 | host, CUDA SMEM / VTCM | 8.1 | M (tangent inexactness); puremath App. B item 20 | I7 |
| M21 | Tree adjoint (six-shape reverse rule in I1-I2; general in I5) | addendum item 21; 11.7; errata Sec. B | host, CUDA | 8.5 | K; G12 | I1-I2, I5 |
| M22 | P0 data generator, baselines (own C++ autograd), evaluator | review Sec. 7; addendum 15.4; companion 15.1 (Evaluator row) | host, CUDA | 8.5 | L (evaluation outcomes); J (literature); G25 | I1-I4, I5-I7 |
| M23 | Adapter boundary (the engineer's existing adapter) | spec 6.2, 6.3; errata E-5; companion 15.1 (Export/loader row); bridge G20 | host <-> DSP / Adreno | 8.7, 8.4 | B, C, D; G20 | I0, I3 |
| M24 | Spectral, RDM, Gram and non-unitary readouts (limits kernels) | limits A1-A5, Table B B1 | host, CUDA, HVX | 8.9 | M | I7 |

PRL ids per module (generated from the PRL):

| Module | PRL ids |
|---|---|
| M0 | FND-01, FND-02, FND-04, FND-07, FND-08, TST-01, TST-02, ERR-01, ERR-03, TST-04, QC-08, DEP-01, DEP-02, DEP-04, DEP-05, DEP-06, CAP-05, DSC-15 |
| M1 | P0-04, ERR-07, GRM-03, GRM-07, GRM-08 |
| M2 | ERR-06, QC-03, QC-07 |
| M3 | REF-04, REF-17, CAP-07 |
| M4 | REF-01, REF-02, REF-03, REF-10 |
| M5 | REF-05, REF-08, ERR-02, CAP-04 |
| M6 | REF-06, GRM-15 |
| M7 | REF-07, REF-09, ERR-04, CUD-06, QC-06 |
| M8 | REF-12, REF-15, CUD-10, CUD-12, CUD-14, DSC-20 |
| M9 | REF-14 |
| M10 | EXP-01, DEP-08 |
| M11 | FND-06, CUD-01, CUD-02, CUD-04, CUD-05, CUD-07, CUD-15, QC-02, QC-04, QC-05, QC-10, QC-11, QC-12, QC-16, QC-18, GRM-13, GRM-14, DSC-17 |
| M12 | GRM-01, GRM-02, GRM-05 |
| M13 | GRM-04, GRM-11 |
| M14 | GRM-09, GRM-12 |
| M15 | GRM-16, CAP-10 |
| M16 | GRM-17, GRM-22 |
| M17 | CAP-01, CAP-03 |
| M18 | CAP-09, CAP-11, CAP-12, CAP-15, CAP-16 |
| M19 | DSC-01, DSC-02, DSC-05 |
| M20 | DSC-08, DSC-09, DSC-11 |
| M21 | REF-13, CUD-09, GRM-10, GRM-18 |
| M22 | P0-01, P0-03, P0-05, P0-06, P0-07, P0-08, P0-09, P0-10, P0-13, P0-12, QC-14, P0-14, P0-15, P0-16, P0-17, P0-18, P0-19, P0-20, P0-22, DEP-03, DEP-10, GRM-19, GRM-20, CAP-06, CAP-13, DSC-14, DSC-19 |
| M23 | FND-03, QC-01, QC-13, DEP-07 |
| M24 | DSC-03, DSC-04, DSC-06, DSC-07, DSC-10, DSC-12, DSC-13, DSC-18 |

### 6.2 Interfaces

| Id | From -> to | Content (with source) |
|---|---|---|
| IF-1 | Host front end -> evaluator | Word ids (2-4 B each, spec 6.3 l1139); cups[] at 2 B per cup and tree[] at 14 B per word (addendum C16 l38-56; Sec. 16 row "tree[] + cups[]"); program / bucket id (spec 2.6 l486-511). Parsing never runs on the vector units (chart on host or DSP scalar core, addendum C16 l52). |
| IF-2 | Export file -> device model image | Angle table (uint16 turns, 2 B/param, or Q15 pairs, 4 B/param) plus a word-id -> offset lookup of 0 or 4 B per word; the 20 B records are not loaded (errata E-5 l66-75); header fields per bridge Sec. 3.4. |
| IF-3 | Evaluator -> caller | 2^{q_s} + 1 values per sentence (probabilities and D; 12 B at (1,1) fp32, spec l1166), the certificate 2e/(2 sqrt D - e) (errata E-11), and the status record (gap G20; DEP-07). |
| IF-4 | Trainer internals | Flat parameter vector in occurrence order (spec 5.5; T7 order Alice, Bob, loves); gradient table; Adam state 16 B/param (spec (5.9); errata E-3). |
| IF-5 | Trainer -> export | Format v1 (spec 6.3), v+1 (addendum C15), generator block (addendum 13.7); quantisation k = round(theta/(4 pi) 65536) mod 65536 from the unwrapped fp32 angle (spec l1135). |

### 6.3 The Qualcomm adapter boundary

The adapter is the engineer's existing code; no registered document describes it, so its interface header
(FND-03) is part of this SAD and is tested by QC-01. What crosses the boundary: IF-2 (model image), IF-1 without
the parser (word ids, cups[] / tree[] or a program id), IF-3 (outputs). What the adapter must provide: a VTCM
reservation through the compute-resource manager (spec 3.2 l639-641: VTCM is shared and must be reserved;
assume only a fraction); an HVX context per sentence stream (spec 3.2 l641-642); the vgather path on v65+ or the
sorted-batch fallback (errata E-9 l114); Adreno dispatch for the fiber-per-sentence kernel (spec 3.4); HMX only
if exposed (spec 3.3 l731-734). What stays on the host: parsing, training, export writing, the fp32 reference.
UNVERIFIED facts at this boundary: VTCM per SoC (256 KB - 4 MB on v66-v69, 8 MB on v73+) and retention across
power collapse; HVX contexts per cDSP (2-4); HMX reachability on v68-v79 (spec Sec. 10 items 2-3).

### 6.4 Memory tiers (addendum C14 l17-24, Sec. 16 l1673-1743; spec Sec. 7 with errata E-3, E-4, E-5, E-13)

| Tier | Size (source) | Objects placed there in this plan (figures from the cited rows) |
|---|---|---|
| REGISTER | CUDA thread about 1 KB (<= 255 x 4 B); HVX register file 32 x 128 B = 4 KB (C14) | Fused tensordot (1,1) 16 amplitudes (10 in place) = 128 B fp32 / 64 B Q1.15 (spec l1154); (1,3) 52 amplitudes = 416 / 208 B (addendum 15.3(a) l1557-1558); (2,1) 48 (36) = 384 / 192 B; relative clause 14 (10) / 44 (36) amplitudes (a1682-1683); tree-adjoint training 400 B for the T12 sentence (a545-548); DisCoCirc N = 4, q_n = 1, 16 amplitudes 128 / 64 B |
| VTCM | 8 KB slice unless stated; 8 MB total on v73+ (C14; spec 3.2 l636-639) | Angle table (P0 Model A 352 B, Model B 640 B, review l551-552; 100,000 B at V = 1e4, errata E-3) plus 0-40,000 B offsets (errata E-5); HVX permute-control constants 768 B (spec Sec. 7); lane-coefficient vectors 3 KB / 2.5 KB optional (errata E-13); MPS N = 32, chi = 4: 7,488 B fp32 / 3,744 B fp16 + 1.0 KB scratch (Sec. 16) |
| SMEM | CUDA 48 KB static, 99 / 163 / 227 KB dynamic; Adreno local 32 KB (C14) | Circuit-form training 24 x 2^W B (W = 10: 24 KB; W = 11: 48 KB; spec l625); MPS N = 16, chi = 16: 37.3 KB fp32 capped (Sec. 16) |
| HBM / DDR / host | -- | Training tables 16 x P_total (96 KB at 6000 params, errata E-3); datasets; chart parser 1.375 m^2 B = 11,638 B at m = 92 (host, addendum l105); DisCoCirc N = 8, q_n = 2 exact text 512 KB unless VTCM >= 512 KB (Sec. 16) |

The claim being tested is scoped by errata E-4: persistent parameters are a small weight table (not zero);
materialised objects 0 B; per-sentence working set 40-400 B. P0 tests the working-set part at 128 B / 416 B
(review l563).

### 6.5 Number formats (spec Sec. 4 as corrected by errata E-6, E-7, E-8, E-11)

| Path | Format | Rules (source) | Bound / tolerance (source) |
|---|---|---|---|
| Training (CUDA, host) | fp32 throughout; fp64 reference | Adam 16 B/param, unwrapped theta (spec 5.5); probabilities accumulated in fp32 only (C12, E-8) | Test tolerance 1e-6 vs fp64 (spec l1258); fp32 Ng_max 8333 worst at E <= 0.01 (spec l875-876) |
| HVX inference | Q1.15 int16 state, Q2.14 gates, int32 accumulate | Gate path store (acc + 2^13) >> 14 (spec l810); tensordot store (acc + 2^14) >> 15 (spec l918) with words pre-scaled by S_q15 = 1 - 2^-15 (errata E-6); lane-varying coefficient vectors (errata E-9) | Tensordot worst norm(delta sigma) 1.96e-4 at (1,1), 3.0e-4 at (2,1) (spec l933-937 with E-6); abs(delta P_c) 5.2e-4 at Z = 0.1403, 1.57e-3 at D = 2^-6 at (1,1) (errata l159); test tolerance 2e-3 |
| Adreno, HVX v68+ IEEE | fp16 storage, fp32 compute | Never accumulate probabilities in fp16 (C12 with E-8); FTZ hazard checked on the device (spec l794) | Worst 5 u16 = 2.4e-3; at D = 2^-6 abs(delta P_c) <= 0.0197 (errata l160); test tolerance 3e-3 |
| Readout policy (all paths) | D and N_c in int32 / fp32 | Single floor D >= 2^-6 on both paths with fp32 fallback (errata E-11 l160-163); fp32 default for abs(K) >= 6 (errata E-10 l145) | Certificate 2e/(2 sqrt D - e), valid iff sqrt D > e (errata E-11) |
| Angles | uint16 half-angle turns (or Q15 cos/sin pairs) | Wrap only at export, mod 4 pi (spec 5.6); never fp16 (spec l792) | Quantisation 4.8e-5 rad (spec C5 l41) |
| Excluded | pure fp16, bf16 arithmetic | Never the right choice (spec 4.6 l893-896) | -- |

### 6.6 UNVERIFIED hardware facts each module relies on (from bridge Sec. 10 B-E; spec Sec. 10 items 2-6)

| Fact | Modules | Closing test (PRL) | Timebox |
|---|---|---|---|
| HVX widening-multiply pair lane order and Rt.h half assignment | M5, M11 | T9 ramp (QC-02) | TB6 |
| vdelta control-byte convention; single-opcode permutes | M11 | T9 vdelta controls (QC-02) | TB6 |
| fp16 rounding mode and subnormal flushing on HVX | M5, M11 (fp16 path) | T9 fl(a*b) record (QC-02); QC-10 | TB6 |
| vgather availability (v65+) | M11 | QC-07 sorted-batch equality | TB6 |
| Multiply / permute unit counts (n_mpy = 2), DV occupancy, coefficient-generation cycles (~36 DV per 64 angles) | M2, M11 | QC-08 measured cycles | TB6 |
| Register counts (HVX 27-29 of 32; CUDA ~52 at (1,1)) | M5, M11, M14 | HVX-REG (QC-04), PTXAS (CUD-02, CUD-12) | TB4-TB6 |
| VTCM per SoC, reservation, retention across power collapse | M23 | ADAPT-CONF (QC-01) | TB6 |
| HMX tile formats, accumulation width, reachability | M11 | HMX-TILE (QC-11, Could) | TB6 |
| Adreno wave size, registers per fibre (~16 fp32 derived), native_sin/cos accuracy | M11 | ADRENO-REG, T6-Adreno (QC-16) | TB7 |
| CUDA shared-memory maxima per compute capability; tensor-core accumulation semantics | M11 | CUD-07 (Could); tensor cores never used for probability sums (NFR-03) | TB4 |
| HVX layout and residency for (1,3) (no document gives it; G31) | M11 | T6-(1,3)-HVX (QC-12) | TB7 |


## 7. Development Approach Definition

### 7.1 Testing strategy

Reference-first. The order is fixed: (1) fp64 reference programs already on disk print the expected values
(verify.c and the folders of bridge Sec. 1); (2) the fp64 C reference library of I1 reproduces them under
assertion; (3) every kernel in any other format or on any device is checked against the fp64 reference at the
spec tolerances; (4) experiment metrics (P0 and later) are computed only by kernels that passed (3).

| Level | What | Tests | Tolerance |
|---|---|---|---|
| L0 | Reference programs (read-only ground truth) | verify.c; addendum/t18.c, t18b.c, mix.c; critic/adjtree.c; derivH/, verH/, verE/, refE/, derivF/, verEZ/, second/, verQ15/, bp/, spsa_var.c; limits programs | They print; they do not assert (bridge Sec. 7) |
| L1 | fp64 / fp32 host library under assertion | T1-T8, T10, T-EZ, E-2, E-10, E-12, then T11-T24 as increments arrive | fp64 identities to 1e-12..1e-16 as stated per test; fp32 1e-6 |
| L2 | Device kernels vs L1 | T4, T6, T7, T24 on CUDA; T6(b), T8, T9 on HVX; T6 fp16 on Adreno | fp32 1e-6; fp16-store 3e-3; Q1.15 2e-3 (spec l1258; addendum l1820) |
| L3 | Error-bound conformance | Per-sentence abs(delta P_c) <= certificate 2e/(2 sqrt D - e); D-floor routing | errata E-11 |
| L4 | Experiment metrics | P0-* (Sec. 5.4), later splits | Predeclared thresholds (review l561-563; addendum 15.4) |
| L5 | Reproduction | REPRO: clean-checkout reproduction with two commands | Bit-identical tables where deterministic; seeds recorded |

Plan-level test ids used in the PRL (defined here; sources in brackets):

| Id | Check |
|---|---|
| CI-NOPY, CI-ASCII, CI-HASH | No *.py file and no Python invocation in the tree; every tracked text file ASCII; sha256sum -c against bridge Sec. 1 (bridge header, R13) |
| P0-SIZE, P0-LABEL, P0-SPLIT, P0-PARSE | Sentence count 1,190,592; role-swap label property; split disjointness; greedy exact on P0 (review l549-557) |
| P0-ACC, P0-PFAIL, P0-OPS, P0-WS, P0-DEV, P0-ALL | Accuracy per split per seed; parse-FAIL on paraphrases; counted ops; measured working set; device vs fp32 reference; full metrics table (review l559-563) |
| PTXAS, HVX-REG, ADRENO-REG | Compiler register / spill reports: 0 spills (addendum C14; errata E-9 l108-110) |
| ADAPT-CONF | Adapter conformance: load model image, submit, read back; resident bytes (errata E-5) |
| TRAIN-EQ | Device training steps equal host steps to 1e-6 for the first 10 steps (same seed) |
| T6(b)-CUDA, T6(b)-HVX, T6-fp16, T6-Adreno, T6-(1,3)-HVX, T6-(2,1)-HVX, T4-CUDA, T4-HVX, T7-CUDA, T24(a)-CUDA | The bridge test run on the named device and format at that format's tolerance |
| T12-(2,1), T12-CUDA, T12-HVX, T20-dev | Extensions of T12 / T20 written in this plan's timeboxes (no program exists: bridge G10) |
| MPO-NORM, GEN-SIZE, GEN-GRAD, MPS-RDM, HMX-TILE | Operator norm <= 1 after (12.16); generator block bytes (addendum l1150); generator gradient vs FD 1e-6; MPS RDM vs exact 1e-6; one HMX tile vs fp64 |
| REPRO | Evidence package reproduces every reported number from a clean checkout (companion 16.1) |

Test authorship: the Solution Tester session writes the test from the documents before the Solution Developer
session writes the kernel (Sec. 2.3 rule 3). New expected values need two agreeing programs (TST-01; errata
l5-6).

### 7.2 Tolerances (single table)

| Comparison | Tolerance | Source |
|---|---|---|
| fp32 kernel vs fp64 reference | 1e-6 | spec l1258; addendum l1820 |
| fp16-store kernel vs fp64 | 3e-3 | spec l1258 |
| Q1.15 kernel vs fp64 | 2e-3 | spec l1258 (measured worked example 3.11e-5, errata l87) |
| Bent-wire vs tensordot (six wire-map rows, fp64) | 1e-12 | spec T7 l1279 |
| CRz decomposition residual | < 1e-15 | spec T3 |
| Adjoint vs central FD (h = 1e-6) | 1e-6 | spec T7 |
| Tree adjoint vs Sec. 8 dLoss | 1e-7 | addendum T24(a) |
| THAT orientation (T14); DisCoCirc wire order (T20) | 1e-15; 1e-12 | addendum l1846, l1870 |
| MPS vs exact (T21) | <= 1e-10 | addendum l1874 |
| limits theme kernels | fp32 within 1e-6 of printed values; fp16 lexicon tables 2e-3 | bridge Sec. 7 limits paragraph |
| T9 E-9 sweep | NOT FIXED (gap G9); proposed by erratum in TB9 | errata l121; review l195 |

### 7.3 Environments

| Env | Purpose | Tools (no Python) | Available by |
|---|---|---|---|
| E1 host | fp64 / fp32 reference, harness, data generation, baselines, export | cc / c++ (gcc or clang), CMake or Bazel, ctest, shell, awk, jq, sha256sum | TB0 |
| E2 CUDA | Layout A/B/C kernels, training | nvcc, ptxas -v (-Xptxas -v), compute-sanitizer, nsys / ncu for measurement | TB0 (FND-06) |
| E3 Hexagon | HVX kernels through the adapter | Hexagon SDK (hexagon-clang, simulator), the engineer's adapter, on-device timers; HexagonDepInstrInfo.td for instruction classes (errata E-9) | TB0 check, TB6 use |
| E4 Adreno | fiber-per-sentence kernel | OpenCL or Vulkan compute compiler with register / spill report | TB7 |
| E5 Measurement | Registers, cycles, bytes, latency on device | Compiler reports; on-device timers; no energy claim without a measured instrument (companion 13.3 Energy row) | TB6-TB7 |

### 7.4 Configuration management

  - Repository: src/ (C / C++ / Mojo / CUDA), tests/ (ctest), ref/ (links to the L0 programs by path and hash),
    data/ (generated manifests, hash-pinned), evidence/ (logs, compiler reports, measurement notes), docs/ (the
    registered documents as read-only copies checked by CI-HASH). Branch per PRL id; merge only when the
    definition of done holds; tag per increment (r0.1 ... r1.3).
  - The baseline spec stays byte-identical and hash-pinned (7408d5b7...efe86c; errata l1); CI-HASH blocks any
    change to a pinned document.
  - Errata process (bridge 8.8): a spec change is an appended erratum E-14, E-15, ... keyed to sNNN, with a plain
    C fp64 program and a second independent pass; consequences propagated to bridge Secs. 2.2, 7, 9, 10 and the
    manifest (version bump, new errata hash). The addendum may be revised (FND-08) but is then re-hashed.
  - Registry update (bridge Sec. 12): any new document is registered with sha256sum and wc -lc in bridge Sec. 1 and
    manifest documents[]; manifest version bumped; `jq . qnlp_manifest.json` must succeed. Measurement notes that
    settle UNVERIFIED items are dated and linked from bridge Sec. 10 (bridge 8.7 step 4).
  - Export format versions: v1 (spec 6.3); v+1 (addendum C15) with the version field incremented; proposals for
    new fields (G2, G19) go through a C15 version bump, never a silent layout change (bridge 8.4 step 1b).
  - Data: generator source, lexicon, label function, seeds and manifests hash-recorded at TB8 kick-off (freeze).
  - Toolchains: compiler and SDK versions recorded in every measurement note.

### 7.5 Definition of done

Per requirement (all must hold):
  1. The acceptance criterion in the PRL passes in CI, with the measured value logged beside the expected value.
  2. The test was written by a different session from the implementation, from the documents (Sec. 2.3).
  3. Every number in code comments, logs or reports cites its document line or program (bridge R7).
  4. NFR-01..NFR-05 hold (no Python; ASCII; conventions and form tags; numerical hard rules; fp64 check).
  5. Compiler register / spill report archived for device kernels; residency stated as a target (C14).
  6. UNVERIFIED items touched are updated (dated note) or left explicitly UNVERIFIED.

Per timebox: every Must done by the rule above; Shoulds / Coulds done or explicitly carried / dropped; Timebox
Review Record complete (Sec. 5.3); CI green including CI-HASH; status report issued; demo held.

Per increment: all timeboxes done; the gate criteria of Sec. 5.4 evaluated and recorded; Deployment Plan
executed (Sec. 3.5); Project Review Report (increment) filed.


## 8. Risk log

Probability (P) and impact (I): H / M / L. Owner = DSDM role (interim holder in brackets where the role is
vacant). "Retired in" = the timebox whose close-out retires the risk if its closing test passes; "accepted" =
consciously not closed in this project. Groups: UV = bridge Sec. 10 UNVERIFIED register (15 groups A-O); G =
bridge Sec. 11 gaps G1-G30 plus G31 (Appendix A); PM = puremath Appendix B conjectures and open problems (item
numbers of App. B l3686-3745); PR = project risks.

### 8.1 From the UNVERIFIED register (bridge Sec. 10)

| ID | Description | P | I | Owner | Mitigation | Retired in | Closing test |
|---|---|---|---|---|---|---|---|
| UV-A | lambeq / DisCoPy conventions (theta scaling, CRz form, closing H, ring order, cup pairing, symbol order) | M | L | Technical Coordinator | No weight import (WNT-05); training from scratch unaffected (spec l1296) | accepted | none (would be a T4/T5 comparison) |
| UV-B | HVX facts: unit counts, DV occupancy, vdelta controls, widening lane order, fp16 rounding / subnormals, contexts, VTCM per SoC, coefficient cycles (~36 DV), (4,1)/(3,3) residency | H | H | Technical Coordinator (Technical Advisor VACANT) | T9 before any HVX kernel; ramp tests; register report; dated notes | TB6 | T9, T9-E9, HVX-REG (QC-02..QC-08) |
| UV-C | HMX / Cloud AI 100: tile generation, accumulation width, reachability via the adapter | M | L | Technical Coordinator | Not needed by the recommended kernel (spec 3.3); never used for probability sums | TB6 if QC-11 done, else accepted | HMX-TILE (QC-11) |
| UV-D | Adreno: wave size, registers per fibre, fp16 rate, native_sin/cos accuracy, local-memory bandwidth | M | M | Technical Coordinator | Adreno is Should; tensordot mandatory there (spec l754-757) | TB7 | T6-Adreno, ADRENO-REG (QC-16) |
| UV-E | CUDA: shared-memory maxima, shuffle throughput, tensor-core accumulation, every register count | M | M | Technical Coordinator | ptxas -v on every kernel; no tensor cores for sums | TB5 | PTXAS (CUD-02, CUD-12) |
| UV-F | Error-model status: typical columns are estimates; Var[dP_c/dtheta] = 0.06 and 5 % local minima measured once; FTZ device-dependent; Sec. 4.6 right-hand columns and a708-709 / T23 not retabulated | M | M | Technical Coordinator | Re-run with new seeds; retabulate (DEP-06, CAP-05); fix the T9 tolerance by erratum (DEP-04) | TB13 | DEP-06, CAP-05 programs agreeing |
| UV-G | Design choices open: L, real-only model, nested cups, ansatz 5, L_sat beyond Q <= 3 | M | L | Technical Coordinator | P0 fixes L = 2, ansatz 0 (review l568-569); rest are Coulds | TB13 (partial) | CAP-07 ranks |
| UV-H | Grammar statistics (m ~ 2.3 x words; cycle and greedy-failure rates are CFG figures); tagger size / accuracy; coreference accuracy | M | M | Business Analyst (engineer) | Measure on the P0 paraphrases (GRM-19) and a public set (GRM-08) | TB12 | GRM-19, GRM-08 |
| UV-I | Hybrid front end: hash-bucket accuracy, mixer cost, q_n heuristic, encoder sizes | M | M | Technical Coordinator | Hybrid evaluation (CAP-13) | TB14 | CAP-13 |
| UV-J | Capacity / positioning literature figures (Lorenz, COGS, SCAN, ...), all UNVERIFIED | H | L | Business Visionary (VACANT) | Never quoted as fact; P0 uses own baselines | accepted | none |
| UV-K | Whether training drives Z above 2^{-2 q_n} | M | M | Technical Coordinator | Log D histograms per epoch (CUD-10, P0-15, CAP-06) | TB13 | CAP-06 |
| UV-L | Evaluation outcomes (all expected results are hypotheses) | H | H | Business Sponsor (VACANT) | Predeclared thresholds and falsifiers; G4 | TB8 | G4 record (P0-18) |
| UV-M | limits specifics (Stinespring gradient path, HMX tiles >= 16x16, MPS tangent inexactness, ...) | M | L | Technical Coordinator | limits themes are Should / Could in I7 | TB17 (partial) | DSC-07, DSC-12, DSC-20 |
| UV-N | Companion open items (no implementation / accuracy / energy result; one-command reproduction absent) | H | M | Project Manager (engineer) | P0 report; REPRO | TB9 | REPRO (DEP-01, DEP-02) |
| UV-O | Document-set hygiene: owed addendum edits not applied; a1657 vs a1653 wording; verify.c digits compiler-dependent | M | L | Technical Coordinator | Addendum revision (FND-08); CI-HASH | TB0 (if FND-08 done) | CI-HASH after re-pin |

### 8.2 From the gaps (bridge Sec. 11) and G31

| ID | Description (short) | P | I | Owner | Mitigation | Retired in | Closing test |
|---|---|---|---|---|---|---|---|
| G1 | No cross-document symbol table outside the bridge | L | L | Business Analyst (engineer) | Use bridge Sec. 4 | accepted | -- |
| G2 | No header field for ring order / closing H | M | L | Technical Coordinator | Record in notes; propose C15 bump | TB9 | DEP-08 proposal |
| G3 | ansatz_id 5 undefined | L | L | Technical Coordinator | Not used; Could | TB13 | CAP-07 |
| G4 | Binary encoding of cups[] / tree[] and opcodes | M | M | Technical Coordinator | Define when M13 lands | TB12 | GRM-22 round trip |
| G5 | No nested-cup test vector at q >= 2 | L | L | Technical Coordinator | Component-wise only (flags.bit1 = 0) | TB11 | GRM-15 |
| G6 | Two objects named wire_map | L | L | Technical Coordinator | Names effect_wire_map / text_wire_map (bridge Sec. 4) | TB1 | code review |
| G7 | One packed-Hermitian layout | L | M | Technical Coordinator | Convert at kernel boundary (bridge row 20) | TB15 | DSC-06 |
| G8 | Mixing real-only and complex words | L | L | Technical Coordinator | Whole-model flags.bit2 only | accepted | -- |
| G9 | No T9 E-9 tolerance | H | M | Technical Coordinator | Erratum proposal from measured sweep | TB9 | DEP-04 erratum draft |
| G10 | No assertion harness; T2, T16, T17, T20, T22 lack programs; T3 CRx not coded; no (2,1) relative-clause vector | H | H | Solution Tester (agents) | TST-01, REF-02, REF-03, CAP-01, DSC-01, GRM-11, GRM-12 | TB15 | the named tests |
| G11 | eps for fidelity readouts | L | M | Technical Coordinator | Proposal DSC-06 | TB15 | DSC-06 |
| G12 | Canonical parameter order for tree / DisCoCirc sentences | M | M | Technical Coordinator | Proposal GRM-22 | TB12 | GRM-22 |
| G13 | Qubit numbering for Sec. 11 circuit form | M | L | Technical Coordinator | Proposal GRM-22 | TB12 | GRM-22 |
| G14 | Canonical test list (T24 exists) | L | L | Business Analyst (engineer) | Use bridge list T1-T24 + T-EZ + errata checks | TB0 | PRL |
| G15 | No addendum hash pin outside the bridge | L | M | Technical Coordinator | CI-HASH pins it | TB0 | CI-HASH |
| G16 | Export of imported-weight conventions | L | L | Technical Coordinator | No import (WNT-05) | accepted | -- |
| G17 | Spec l176 relative-pronoun type unlabelled | L | M | Technical Coordinator | Use addendum a169 / a171 types (GRM-03) | TB10 | T11 rows |
| G18 | No tractability-contract record | M | L | Project Manager (engineer) | DEP-10 | TB9 | DEP-10 |
| G19 | Export fields r, chi, readout, grammar version, checksum | M | M | Technical Coordinator | DEP-08 proposal; C15 bump | TB9 | DEP-08 |
| G20 | No output status record struct | H | M | Technical Coordinator | DEP-07 in the adapter header | TB9 | DEP-07 |
| G21 | Sec. 16 lacks system overhead, traffic, string table rows | M | L | Technical Coordinator | Traffic measured in QC-13 | TB7 | QC-13 |
| G22 | Un-applied addendum / companion edits | M | L | Technical Coordinator | FND-08 | TB0 | CI-HASH |
| G23 | Evidence labels exist only in the bridge | M | L | Project Manager (engineer) | Status template uses them (FND-07) | TB0 | status template |
| G24 | No device measurement anywhere | H | H | Technical Coordinator | I2-I3 device work, dated notes | TB7 | PTXAS, HVX-REG, QC-14 |
| G25 | P0 artefacts missing (lexicon, labels, splits, paraphrases, baselines) | H | H | Business Analyst (engineer); Ambassador VACANT | P0-01..P0-16 | TB8 | P0-SIZE .. P0-ALL |
| G26 | Parser scores for ambiguity weights | M | L | Technical Coordinator | GRM-05 accumulators; weights uniform until specified | accepted | -- |
| G27 | Per-profile invariance declarations | M | L | Business Visionary (VACANT) | P0 invariance = role-swap label property | TB3 | P0-LABEL |
| G28 | E-11 recomputation of a709, T23, 14.2, Sec. 4.6 columns | M | M | Technical Coordinator | DEP-06, CAP-05 | TB13 | DEP-06, CAP-05 |
| G29 | Inference acceptance tolerance per deployed profile | M | M | Business Sponsor (VACANT) | Certificate + test tolerances for P0; fix per profile later | TB8 | G4 record |
| G30 | Sum vs Frobenius aggregation for multi-line records | L | L | Technical Coordinator | Only relevant to Won't profiles (WNT-01) | accepted | -- |
| G31 | No HVX layout specified for (1, q_s = 3) (P0 Model B) | H | M | Technical Coordinator | Layout spike in TB7; proposal to addendum owner | TB7 | T6-(1,3)-HVX (QC-12) |

### 8.3 From the pure-mathematics document (App. B conjectures and open problems)

| ID | Description | P | I | Owner | Mitigation | Retired in | Closing test |
|---|---|---|---|---|---|---|---|
| PM-1 | Conjecture 1.26: characterisation of greedy failure (App. B item 1) | L | L | Technical Coordinator | Greedy is pre-check only; chart in production (errata E-12) | TB10 | E-12 regression (GRM-02) |
| PM-8 | Conjecture 2.34: spider realisation in the bent-wire circuit with 2^{-q/2} per leg / cup (item 8) | M | M | Technical Coordinator | Circuit form is not the training path for Sec. 11 sentences (addendum 11.5); check numerically | TB12 | T18 plus a numeric spider-circuit check in GRM-17 |
| PM-9 | Which cup pairing the reference implementation uses (item 9) | L | L | Technical Coordinator | Component-wise fixed (C8, flags.bit1 = 0) | accepted | -- |
| PM-14 | IQP mean Z = 0.041 at (2,1) unexplained (item 14) | M | M | Technical Coordinator | Z_min guard load-bearing at q_n >= 2 (errata E-1); fp32 fallback | TB13 | CAP-06 histograms |
| PM-15 | Training raises Z above 2^{-2 q_n} (item 15) | M | M | Technical Coordinator | Same as UV-K | TB13 | CAP-06 |
| PM-19 | Treewidth of word graphs for general lexica (item 19) | M | M | Technical Coordinator | Peak by (11.18') measured per sentence; T22 | TB11 | GRM-11 |
| PM-20 | Effect of inexact SVD tangents in MPS training (item 20) | M | M | Technical Coordinator | MPS used forward; training through MPS not planned | accepted | -- |
| PM-22 | W_train growth law heuristic (item 22) | L | L | Technical Coordinator | Tree adjoint is the training path (addendum 11.7) | accepted | -- |
| PM-25 | No barren plateau for the normalised readout as W grows (item 25) | M | H | Technical Coordinator | Log Var[dP_c/dtheta] per profile (errata E-10 "re-measure for every new profile") | TB13 | CAP-06 logs |
| PM-26 | Saddle at the origin for every IQP profile beyond (1,1) and (2,1) (item 26) | M | M | Technical Coordinator | Uniform init only (errata E-10); non-zero generator bias (CAP-11) | TB14 | CAP-11, E-10 tests |
| PM-28 | Sharper sincos table bound 6.03e-5 < 2^-14 not reproduced (item 28) | L | L | Technical Coordinator | Use the polynomial (spec 4.4) or exported pairs | TB6 | QC-07 |
| PM-29 | Behaviour under flush-to-zero (item 29) | M | M | Technical Coordinator | FTZ off for state kernels (spec Sec. 9 item 11); record on device | TB6 | T9 fl(a*b) record |
| PM-31 | Multi-sentence amplification 1/sqrt(prod p_t) (item 31) | M | M | Technical Coordinator | DisCoCirc readout without postselection across texts where possible (addendum l968-972) | TB17 | DSC-14 D logs |
| PM-33 | Conjecture 7.13: generic ranks / surjectivity of Sim14 / Sim15 at L_sat (item 33) | L | M | Technical Coordinator | P0 uses IQP L = 2; ranks checked (CAP-07) | TB13 | CAP-07 |
| PM-37 | Generalisation / retrieval / probe figures of addendum 15.3 (item 37) | H | M | Business Visionary (VACANT) | Treated as hypotheses; P0 and DSC-14 measure | TB17 | P0-ALL, DSC-14 |

### 8.4 Project risks

| ID | Description | P | I | Owner | Mitigation | Retired in | Closing test |
|---|---|---|---|---|---|---|---|
| PR-01 | Solo team: illness or reassignment stops the programme (bus factor 1) | M | H | Business Sponsor (VACANT) | Evidence package and one-command reproduction early (TB9); bridge / plan let an agent or new engineer resume | TB9 (reduced) | REPRO |
| PR-02 | Business roles vacant: self-certified gates; nobody can stop the programme | H | H | Business Sponsor (VACANT) | Predeclared numeric gates; external signature at G4; recommendations of Sec. 2.2 | TB8 | signed G4 record |
| PR-03 | Hardware access: no v65+ Hexagon part or GPU when needed | M | H | Technical Coordinator | Check in TB0; sorted-batch fallback (errata E-9 l114); simulator for correctness only; P0 on CUDA evidence if needed (G3 fail path) | TB6 | T9 on a named part |
| PR-04 | Agent usage limits remove capacity mid-timebox | H | M | Project Manager (engineer) | Agents planned at 4 of 10 ideal days; Coulds / Shoulds absorb loss; usage budget tracked daily | continuous | TB review "agent usage" field |
| PR-05 | Agent-written tests share the implementer's misreading | M | H | Solution Tester (agents) | Separate tester session from documents only; second independent pass for new numbers; tests cite lines | continuous | TST-01 rule audit at each close-out |
| PR-06 | The adapter (not described by any document) lacks a needed capability (VTCM reservation, vgather path, Adreno dispatch) | M | H | Technical Coordinator | Conformance test QC-01 first in TB6; adapter changes planned as Shoulds | TB6 | ADAPT-CONF |
| PR-07 | Estimates wrong (no historical velocity) | H | M | Project Manager (engineer) | Calibration spike (FND-08); recalibrate after TB2; MoSCoW contingency 40 % | TB2 | velocity field in TB2 review |
| PR-08 | Sunk-cost continuation after a falsified P0 | M | H | Business Sponsor (VACANT) | G4 decision table fixed at G0; STOP is the default | TB8 | G4 record |
| PR-09 | Scope creep into deferred companion profiles | M | M | Business Visionary (VACANT) | Won't list WNT-01..WNT-09; c450 condition | continuous | PRL review at each close-out |
| PR-10 | Document drift: a pinned document changes silently | L | H | Technical Coordinator | CI-HASH; errata process | TB0 | CI-HASH |
| PR-11 | Data leakage between P0 splits | M | H | Business Advisor (VACANT) | Split checks (P0-SPLIT); advisor review in TB3 | TB3 | P0-SPLIT |
| PR-12 | Paraphrase set written by the engineer biases parse-FAIL downward | H | M | Business Ambassador (VACANT) | Independent author (P0-14); recorded authorship | TB8 | P0-14 record |
| PR-13 | Toolchain churn (CUDA, Hexagon SDK) breaks reproduction | M | M | Technical Coordinator | Versions pinned in measurement notes; container recipe as a Could | TB9 | REPRO |
| PR-14 | "Measured working set" has no agreed measurement protocol | M | H | Technical Coordinator | Define at G0: resident state bytes from the compiler's register / local-memory report plus declared VTCM scratch per sentence; recorded in the ToR | TB7 | P0-WS (QC-14) |

Top risks (H x H): UV-B (HVX facts), UV-L (evaluation outcomes), G10 (no harness), G24 (no device measurement),
G25 (P0 artefacts), PR-02 (vacant business roles). All six are retired by Week 18 if the plan holds.


## 9. Traceability matrix

Requirement -> document section -> test -> timebox (weeks) -> module. Generated from the PRL source so that it
cannot drift from Sec. 4; module names in Sec. 6.1; test ids in the bridge test registry (bridge Sec. 7) or Sec.
7.1. Cross-cutting NFR-01..NFR-05 apply to every row.

| Req | MoSCoW | Document section | Test(s) | TB (weeks) | Module |
|---|---|---|---|---|---|
| FND-01 | M | review Sec. 7 l547-576; companion 13.5 c735-746, 15 c857-875 | - | TB0 (W1-2) | M0 |
| FND-02 | M | DSDM MoSCoW rule; bridge Sec. 7 | - | TB0 (W1-2) | M0 |
| FND-03 | M | spec Sec. 9 l1239-1256; addendum Sec. 18 l1790-1813; spec 6.2 l1100-1110 | - | TB0 (W1-2) | M23 |
| FND-04 | M | bridge header, Sec. 0 hard rules, Sec. 1, R13; errata l1; companion 16.1 c895-910 | CI-NOPY, CI-ASCII, CI-HASH | TB0 (W1-2) | M0 |
| FND-06 | S | review l564-565; errata E-9 l114; spec Sec. 10 items 2, 6 | - | TB0 (W1-2) | M11 |
| FND-07 | S | bridge Secs. 10-11, G23; puremath App. B l3686-3745; companion 1.3 c66-77 | - | TB0 (W1-2) | M0 |
| FND-08 | C | bridge Sec. 10 item O, G22; errata E-4, E-9, E-11, E-12; this plan Sec. 10 | CI-HASH | TB0 (W1-2) | M0 |
| REF-01 | M | spec 2.1-2.2 l356-413; spec T1 l1259 | T1 | TB1 (W3-4) | M4 |
| REF-02 | M | spec C5-C6 l32-60, C13 l115-142, (0.1)-(0.8); bridge G10 | T2 | TB1 (W3-4) | M4 |
| REF-03 | M | spec T3 l1263; bridge Sec. 7 T3 | T3 | TB1 (W3-4) | M4 |
| REF-04 | M | spec C4 l28, C7 l62-71, C10 l86-102 | T4 | TB1 (W3-4) | M3 |
| REF-05 | M | spec 1.3 l225-267; errata E-13 l194-204 | T5, T6(a)(b) | TB1 (W3-4) | M5 |
| REF-06 | M | spec 1.4 l268-328, 2.3 l414-464 | T6(c)(d), T7-wiremap | TB1 (W3-4) | M6 |
| REF-07 | M | spec 1.5 (1.6)-(1.7) l329-340; spec 5.1 l961-971 | T5, T6 | TB1 (W3-4) | M7 |
| TST-01 | M | bridge G10, 8.8 step 3; spec l1258; errata l5-6; companion 16.1 c897 | T1-T6 | TB1 (W3-4) | M0 |
| REF-08 | S | spec 2.4 l465-473, 2.5 l474-485 | T6(a) | TB1 (W3-4) | M5 |
| REF-09 | S | spec 1.5 l335; limits A3 l591-611 | A3-gram | TB1 (W3-4) | M7 |
| TST-02 | S | bridge Sec. 0 step 4; bridge Sec. 1 verify_c row | T4-T7 (verify.c) | TB1 (W3-4) | M0 |
| REF-10 | C | spec 4.3 l799-829, 2.3 l414-464 | T8-Q31, T6(d) | TB1 (W3-4) | M4 |
| REF-12 | M | spec 5.3 l1009-1043; review l320-321; bridge R11 | T7 | TB2 (W5-6) | M8 |
| REF-13 | M | errata Sec. B l210-211; addendum 11.7 (11.27')-(11.30) l501-545 | T24(a) | TB2 (W5-6) | M21 |
| ERR-01 | M | errata E-7 l89-93, E-8 l95-98; spec C12 l110-113 | T8-E7 | TB2 (W5-6) | M0 |
| ERR-02 | M | errata E-6 l77-87; spec l810, l918 | T8-E6 | TB2 (W5-6) | M5 |
| EXP-01 | M | spec 6.3 l1111-1142; errata E-3, E-5; bridge Sec. 3.4 | T10, E-5 | TB2 (W5-6) | M10 |
| REF-15 | M | spec 5.5 l1054-1075; errata E-10 l126-147 | E-10 | TB2 (W5-6) | M8 |
| REF-14 | S | spec 5.2 l972-1008, 5.6 l1076-1087; errata E-2 l30-39 | T7-shift, E-2 | TB2 (W5-6) | M9 |
| ERR-04 | S | errata E-11 l149-165, E-10 l145 | T8-E11 | TB2 (W5-6) | M7 |
| ERR-06 | C | spec 4.4 l830-843; errata E-9 l100-124; review l195 | T9-codec, T9-E9 (host) | TB2 (W5-6) | M2 |
| REF-17 | C | spec C3 l22, C10 l86-102, Sec. 10 item 8 | T4-real | TB2 (W5-6) | M3 |
| ERR-03 | M | errata E-1 l13-28 | T-EZ | TB3 (W7-8) | M0 |
| P0-01 | M | review Sec. 7 l549-550, l554-555; bridge G25 | P0-SIZE, P0-LABEL | TB3 (W7-8) | M22 |
| P0-03 | M | review l555-557; bridge G25 | P0-SPLIT | TB3 (W7-8) | M22 |
| P0-04 | M | spec 1.1 l146-194, 2.6 l486-511, Sec. 9 item 1; errata E-12 | T5, T6, P0-PARSE | TB3 (W7-8) | M1 |
| P0-05 | M | review l550-552 | P0-ACC | TB3 (W7-8) | M22 |
| P0-06 | M | review l558-559; spec 1.6 l341-353; companion c450 | P0-ACC | TB3 (W7-8) | M22 |
| P0-07 | M | review l562-563; this plan Sec. 5.4 | G1 | TB3 (W7-8) | M22 |
| P0-08 | S | review l557; addendum 15.4 item 4 l1634-1642 | P0-ACC | TB3 (W7-8) | M22 |
| P0-09 | S | review l552-553; addendum 15.3(a) l1553-1558 | P0-ACC | TB3 (W7-8) | M22 |
| ERR-07 | S | errata E-12 l167-192 | E-12 (greedy) | TB3 (W7-8) | M1 |
| P0-10 | C | errata E-10 l137, E-1 l19; review l560 | E-10 (histogram), P0-OPS | TB3 (W7-8) | M22 |
| TST-04 | C | companion 16.1 c901-902 | - | TB3 (W7-8) | M0 |
| CUD-01 | M | spec 3.1 l514-545, C5 l39-45, Sec. 9 items 2-3; review l569 | T4-CUDA, T6(b)-CUDA | TB4 (W9-10) | M11 |
| CUD-02 | M | spec 3.1 l527-540; addendum C14 l17-24; review l553 | PTXAS | TB4 (W9-10) | M11 |
| CUD-04 | M | spec 3.1 l541-545, 6.1 l1090-1099; limits Table B B11 | T6-CUDA-batch | TB4 (W9-10) | M11 |
| CUD-05 | M | spec 3.1 l546-575; spec 6.1 | T6(c)-CUDA | TB4 (W9-10) | M11 |
| CUD-06 | S | errata E-11 l160-163; spec 6.2 | T8-E11-CUDA | TB4 (W9-10) | M7 |
| P0-13 | S | review l558; addendum 15.4 item 4 | P0-ACC | TB4 (W9-10) | M22 |
| CUD-07 | C | spec 3.1 l546-630; errata E-13 | T6(c)-CUDA | TB4 (W9-10) | M11 |
| CUD-09 | M | review l569; addendum 11.7; errata Sec. B l210-211 | T24(a)-CUDA | TB5 (W11-12) | M21 |
| CUD-10 | M | spec 5.5, 6.1 l1090-1099, 6.3; errata E-10 l137-146; bridge 8.4 | TRAIN-EQ, T10 | TB5 (W11-12) | M8 |
| P0-12 | M | review l557; addendum 15.4 item 4 (B2) | P0-ACC | TB5 (W11-12) | M22 |
| CUD-12 | S | spec 5.3; spec T7 l1273; addendum C14 | T7-CUDA, PTXAS | TB5 (W11-12) | M8 |
| CUD-14 | C | spec 4.6 l893-899 | - | TB5 (W11-12) | M8 |
| CUD-15 | C | companion 13.3 Runtime row | - | TB5 (W11-12) | M11 |
| QC-01 | M | spec 6.2 l1100-1110, 6.3 l1137-1139; errata E-5 l66-75; spec 3.2 l637-641 | ADAPT-CONF | TB6 (W13-14) | M23 |
| QC-02 | M | spec T9 l1283; spec 3.2 l658-663 | T9 | TB6 (W13-14) | M11 |
| QC-03 | M | errata E-9 l121; review l195 | T9-E9 | TB6 (W13-14) | M2 |
| QC-04 | M | errata E-9 l100-124, E-6; spec 4.7 l902-958; bridge 8.1a | HVX-REG | TB6 (W13-14) | M11 |
| QC-05 | M | spec T6, T8; errata E-6 | T6(b)-HVX, T8-HVX | TB6 (W13-14) | M11 |
| QC-06 | M | errata E-11 l160-163; spec 6.2 | T8-E11-HVX | TB6 (W13-14) | M7 |
| QC-07 | S | errata E-9 l112-114; spec 6.3 l1132 | T10, T4-HVX | TB6 (W13-14) | M2 |
| QC-08 | S | bridge 8.7 step 4, 8.1a step 4 | HVX-REG | TB6 (W13-14) | M0 |
| QC-10 | C | errata E-9 l115-116; spec 3.2 l643-654 | T6-fp16 | TB6 (W13-14) | M11 |
| QC-11 | C | spec 3.3 l719-739; bridge Sec. 10 C | HMX-TILE | TB6 (W13-14) | M11 |
| QC-12 | M | review l552-553; spec 3.2 l709-718; this plan G31 | T6-(1,3)-HVX | TB7 (W15-16) | M11 |
| QC-13 | M | review l570-571; errata E-11, E-4 (c); spec 6.3 l1138-1139 | P0-DEV | TB7 (W15-16) | M23 |
| QC-14 | M | review l559-560, l563 | P0-WS | TB7 (W15-16) | M22 |
| QC-16 | S | spec 3.4 l741-757; bridge Sec. 10 D | T6-Adreno, ADRENO-REG | TB7 (W15-16) | M11 |
| QC-18 | C | spec 3.2 l714-718; review l198 | T6-(2,1)-HVX | TB7 (W15-16) | M11 |
| P0-14 | M | review l556 | P0-PFAIL | TB8 (W17-18) | M22 |
| P0-15 | M | review l557; errata E-10 l141-142 | P0-ACC | TB8 (W17-18) | M22 |
| P0-16 | M | review l557-559 | P0-ACC | TB8 (W17-18) | M22 |
| P0-17 | M | review l559-561 | P0-ALL | TB8 (W17-18) | M22 |
| P0-18 | M | review l561-563 | G4 | TB8 (W17-18) | M22 |
| P0-19 | S | companion 13.5 item 4 c742 | P0-ALL | TB8 (W17-18) | M22 |
| P0-20 | S | companion 8.4 c451, c454; spec C10 | P0-ACC, P0-DEV | TB8 (W17-18) | M22 |
| P0-22 | C | addendum 15.4 items 1, 3 l1614-1624; 15.3(a) | - | TB8 (W17-18) | M22 |
| DEP-01 | M | companion 16.1 c895-910; bridge G25 | REPRO | TB9 (W19-20) | M0 |
| DEP-02 | M | companion 16.1 c897-899; bridge G10 | REPRO | TB9 (W19-20) | M0 |
| DEP-03 | M | companion 16.3 c918-921; addendum 15.4 item 5 | - | TB9 (W19-20) | M22 |
| DEP-04 | M | bridge 8.7 step 4, 8.8, Sec. 10 | CI-HASH | TB9 (W19-20) | M0 |
| DEP-05 | M | this plan Secs. 3.5, 12 | - | TB9 (W19-20) | M0 |
| DEP-06 | S | errata E-11 l149-165; bridge G28 | - | TB9 (W19-20) | M0 |
| DEP-07 | S | bridge G20; companion 7.6 c410-414 | - | TB9 (W19-20) | M23 |
| DEP-08 | C | bridge G2, G16, G19; spec C10 l99; review l402 | - | TB9 (W19-20) | M10 |
| DEP-10 | C | companion 6.2 c298-308; bridge G18 | - | TB9 (W19-20) | M22 |
| GRM-01 | M | addendum 11.1 l80-140, Sec. 18 item 12; errata E-12 | T11 | TB10 (W21-22) | M12 |
| GRM-02 | M | errata E-12 l186-188 | E-12 | TB10 (W21-22) | M12 |
| GRM-03 | M | addendum 11.2 l141-189, C15 l26-36; bridge 8.2 | T11 | TB10 (W21-22) | M1 |
| GRM-04 | M | addendum 11.4 l253-401, C16 l38-56, Sec. 18 item 13 | T12-tree | TB10 (W21-22) | M13 |
| GRM-05 | S | addendum 11.6, l105; bridge G26 | T11 | TB10 (W21-22) | M12 |
| GRM-07 | S | errata E-12; addendum 11.1 | E-12 | TB10 (W21-22) | M1 |
| GRM-08 | C | addendum 11.6; bridge Sec. 10 H | - | TB10 (W21-22) | M1 |
| GRM-09 | M | addendum 11.4 eval_tree, (11.14)-(11.17'), Sec. 18 item 14 | T12, T13, T14 | TB11 (W23-24) | M14 |
| GRM-10 | M | addendum 11.7 l501-571, Sec. 18 item 21 | T24(b)-(e) | TB11 (W23-24) | M21 |
| GRM-11 | M | addendum T22 l1876; bridge G10, conflict row 36 | T22 | TB11 (W23-24) | M13 |
| GRM-12 | S | bridge G10, 8.2 step 3 | T12-(2,1) | TB11 (W23-24) | M14 |
| GRM-13 | S | addendum Sec. 16 a1682-1683 | T12-CUDA | TB11 (W23-24) | M11 |
| GRM-14 | C | addendum Sec. 16; errata E-9 | T12-HVX | TB11 (W23-24) | M11 |
| GRM-15 | C | bridge G5; spec C8 l73-80 | - | TB11 (W23-24) | M6 |
| GRM-16 | M | addendum C15 l26-36, Sec. 18 item 15 | T10-v2 | TB12 (W25-26) | M15 |
| GRM-17 | M | addendum 11.5 l402-451, Sec. 18 item 16 | T18 | TB12 (W25-26) | M16 |
| GRM-18 | M | addendum 11.7; Sec. 16 training rows | T24(b)-CUDA | TB12 (W25-26) | M21 |
| GRM-19 | M | addendum 15.4 items 1, 5 | P0-PFAIL | TB12 (W25-26) | M22 |
| GRM-20 | S | addendum 15.4 item 3 l1624-1627; companion 8.4 c453 | - | TB12 (W25-26) | M22 |
| GRM-22 | C | bridge G4, G12, G13; addendum C16 | - | TB12 (W25-26) | M16 |
| CAP-01 | M | addendum 12.5 l755-822, (12.16') l807-818, Sec. 18 item 17 | T16, T17 | TB13 (W27-28) | M17 |
| CAP-03 | M | addendum 12.5 (12.15)-(12.16) | MPO-NORM | TB13 (W27-28) | M17 |
| CAP-04 | M | addendum 12.3 l672-740; T23 l1878; errata E-11 | T23 | TB13 (W27-28) | M5 |
| CAP-05 | S | bridge G28, conflict row 39 | T23 | TB13 (W27-28) | M0 |
| CAP-06 | S | bridge Sec. 10 K; puremath App. B item 15 | - | TB13 (W27-28) | M22 |
| CAP-07 | C | addendum l586, l1201-1203; puremath App. B item 33; bridge Sec. 10 G, G3 | - | TB13 (W27-28) | M3 |
| CAP-09 | M | addendum 13.1 l881-949, Sec. 18 item 18 | T19 | TB14 (W29-30) | M18 |
| CAP-10 | M | addendum 13.7 l1123-1166; C15 | T19, GEN-SIZE | TB14 (W29-30) | M15 |
| CAP-11 | M | errata E-10 l147; addendum (13.1) | E-10 | TB14 (W29-30) | M18 |
| CAP-12 | M | addendum 13.4 l1045-1069 | GEN-GRAD | TB14 (W29-30) | M18 |
| CAP-13 | S | addendum 15.4, Sec. 16 (13.4') rows; bridge Sec. 10 I | - | TB14 (W29-30) | M22 |
| CAP-15 | C | addendum 13.2, Sec. 16 context row | - | TB14 (W29-30) | M18 |
| CAP-16 | C | addendum 13.5 l1070-1091 | - | TB14 (W29-30) | M18 |
| DSC-01 | M | addendum C17 l58-70, 14.1 l1174-1243, Sec. 18 item 19 | T20 | TB15 (W31-32) | M19 |
| DSC-02 | M | addendum (14.19) l1402-1403; bridge 8.9 | T-W3 | TB15 (W31-32) | M19 |
| DSC-03 | M | limits A2 l332-480; bridge conflict rows 41-42 | T-SF1..4 | TB15 (W31-32) | M24 |
| DSC-04 | M | limits A4 l626-793; bridge 8.9 | T-W4 | TB15 (W31-32) | M24 |
| DSC-05 | S | addendum 14.5 l1378-1397; bridge Sec. 10 H | - | TB15 (W31-32) | M19 |
| DSC-06 | S | addendum 14.6 l1425; bridge G7, G11 | - | TB15 (W31-32) | M24 |
| DSC-07 | C | limits A1 l173-331 | A1-T1..T5b | TB15 (W31-32) | M24 |
| DSC-08 | M | addendum 14.3 l1270-1355, Sec. 18 item 20 | T15, T21 | TB16 (W33-34) | M20 |
| DSC-09 | M | addendum (14.19') l1405-1415 | MPS-RDM | TB16 (W33-34) | M20 |
| DSC-10 | S | limits A3 l481-625 | A3-gram | TB16 (W33-34) | M24 |
| DSC-11 | S | addendum Sec. 16 MPS rows | - | TB16 (W33-34) | M20 |
| DSC-12 | C | limits A5 l794-952 | A5 A-H | TB16 (W33-34) | M24 |
| DSC-13 | C | limits Table B B1; limits Theme B l1369-1468 | B-parse | TB16 (W33-34) | M24 |
| DSC-14 | M | addendum 15.4 item 2(iii) l1618-1621 | - | TB17 (W35-36) | M22 |
| DSC-15 | M | companion 16.1; this plan Secs. 3.5, 12 | REPRO | TB17 (W35-36) | M0 |
| DSC-17 | S | addendum Sec. 16 DisCoCirc rows; 14.1 l1229-1236 | T20-dev | TB17 (W35-36) | M11 |
| DSC-18 | S | limits A4 l737-780 | A4-grad | TB17 (W35-36) | M24 |
| DSC-19 | C | addendum 15.4 item 3 | - | TB17 (W35-36) | M22 |
| DSC-20 | C | limits A6 l953-1100 | A6-et | TB17 (W35-36) | M8 |

Coverage check (computed): every one of the 138 schedulable requirements has a document source, a timebox and a module (checked: 0 rows missing any of them). Bridge test registry ids and the requirements that execute them:

| Test id | Requirement(s) (timebox) |
|---|---|
| T1 | REF-01 (TB1), TST-01 (TB1) |
| T2 | REF-02 (TB1) |
| T3 | REF-03 (TB1) |
| T4 | REF-04 (TB1), TST-02 (TB1), REF-17 (TB2), CUD-01 (TB4), QC-07 (TB6) |
| T5 | REF-05 (TB1), REF-07 (TB1), P0-04 (TB3) |
| T6 | REF-05 (TB1), REF-06 (TB1), REF-07 (TB1), REF-08 (TB1), REF-10 (TB1), P0-04 (TB3), CUD-01 (TB4), CUD-04 (TB4), CUD-05 (TB4), CUD-07 (TB4), QC-05 (TB6), QC-10 (TB6), QC-12 (TB7), QC-16 (TB7), QC-18 (TB7) |
| T7 | REF-06 (TB1), REF-12 (TB2), REF-14 (TB2), CUD-12 (TB5) |
| T8 | REF-10 (TB1), ERR-01 (TB2), ERR-02 (TB2), ERR-04 (TB2), CUD-06 (TB4), QC-05 (TB6), QC-06 (TB6) |
| T9 | ERR-06 (TB2), QC-02 (TB6), QC-03 (TB6) |
| T10 | EXP-01 (TB2), CUD-10 (TB5), QC-07 (TB6), GRM-16 (TB12) |
| T-EZ | ERR-03 (TB3) |
| E-2 | REF-14 (TB2) |
| E-5 | EXP-01 (TB2) |
| E-10 | REF-15 (TB2), P0-10 (TB3), CAP-11 (TB14) |
| E-12 | ERR-07 (TB3), GRM-02 (TB10), GRM-07 (TB10) |
| T11 | GRM-01 (TB10), GRM-03 (TB10), GRM-05 (TB10) |
| T12 | GRM-04 (TB10), GRM-09 (TB11), GRM-10 (TB11), GRM-12 (TB11), GRM-13 (TB11), GRM-14 (TB11), GRM-18 (TB12) |
| T13 | GRM-09 (TB11) |
| T14 | GRM-09 (TB11) |
| T15 | DSC-08 (TB16) |
| T16 | CAP-01 (TB13) |
| T17 | CAP-01 (TB13) |
| T18 | GRM-17 (TB12) |
| T19 | CAP-09 (TB14), CAP-10 (TB14) |
| T20 | DSC-01 (TB15), DSC-17 (TB17) |
| T21 | DSC-08 (TB16) |
| T22 | GRM-11 (TB11) |
| T23 | CAP-04 (TB13), CAP-05 (TB13) |
| T24 | REF-13 (TB2), CUD-09 (TB5), GRM-10 (TB11), GRM-18 (TB12) |
| T-W3 | DSC-02 (TB15) |
| T-W4 | DSC-04 (TB15) |
| T-SF1..4 | DSC-03 (TB15) |
| A1-T1..T5b | DSC-07 (TB15) |
| A3-gram | REF-09 (TB1), DSC-10 (TB16) |
| A4-grad | DSC-18 (TB17) |
| A5 | DSC-12 (TB16) |
| A6-et | DSC-20 (TB17) |
| B-parse | DSC-13 (TB16) |


## 10. Estimation

### 10.1 Basis

Effort is in ideal days (one person-day of uninterrupted work, whether typed by the engineer or by an agent and
reviewed by the engineer). The estimates are expert judgement by size class, anchored on one fact that lowers
the risk of this programme: for almost every requirement the EXPECTED VALUES already exist in a reference
program or document (bridge Sec. 7), so the work is implementation plus test, not derivation.

| Class | Ideal days | Typical item | Uncertainty (one sigma, judgement) |
|---|---|---|---|
| XS | 0.5 | One helper plus its printed reference values (REF-01, ERR-01, ERR-03) | +-50 % |
| S | 1.0 | A module part with 1-2 tests (REF-04, EXP-01, GRM-03) | +-50 % |
| M | 1.5 | A device variant of an existing kernel, a baseline model, a new front-end component | +-60 %; device items +100 % / -30 % |
| L | 2.0 | A new kernel family, a training path, a new parser (CUD-09, GRM-01, CAP-09, DSC-01) | +-70 % |
| XL | 2.5-3.0 | A new algorithm with several tests (GRM-09 node ops, DSC-08 MPS, CUD-01 with device word states) | +-80 % |

No historical velocity exists (risk PR-07). The FND-08 calibration and the TB1-TB2 review records replace these
judgements with measured throughput from TB3 onward.

### 10.2 Velocity assumption (one engineer plus agents)

| Source of capacity | Ideal days per 10-day timebox | Assumption |
|---|---|---|
| Engineer, direct development | 6.0 | 10 working days less about 1 day of DSDM ceremony (kick-off, stand-ups, close-out), about 2 days reviewing agent output, about 1 day of communication and administration |
| Agents (accepted output, after review) | 4.0 | UNVERIFIED; bounded by usage limits (risk PR-04) and by the engineer's review time; calibrated by FND-08 |
| Planning capacity | 10.0 | Per timebox |

Robustness: the largest Must load in any timebox is 6.0 ideal days (TB1, TB2, TB3, TB15), equal to the engineer's
direct capacity. If agent capacity drops to zero, every timebox can still deliver its Musts by dropping all its
Shoulds and Coulds; the dates hold and only contingency scope is lost.

### 10.3 Totals, duration and contingency

| Scope | Effort (ideal days) | Musts | Shoulds | Coulds | Duration |
|---|---|---|---|---|---|
| I0-I4 (up to the P0 decision and its Deployment) | 92.0 | 52.5 | 20.0 | 19.5 | Weeks 1-20 (10 timeboxes) |
| I5-I7 (only if G4 passes) | 68.5 | 36.5 | 16.0 | 16.0 | Weeks 21-36 (8 timeboxes) |
| Whole plan | 160.5 | 89.0 (55 %) | 36.0 (22 %) | 35.5 (22 %) | 38 weeks nominal incl. final Deployment (Weeks 37-38) |

Planning capacity over 18 timeboxes is 180 ideal days; the plan loads 160.5 (89 %), leaving 19.5 ideal days of
unallocated slack in addition to the Shoulds and Coulds.

Contingency: per timebox, the Musts (<= 60 %) can overrun by up to 67 % before any Must is at risk, because all
Shoulds and Coulds of that timebox can be dropped. Across the project, Shoulds and Coulds are 71.5 ideal days
(45 % of the effort).

Uncertainty, stated plainly: the nominal 38 weeks is a P50-style planning figure, not a forecast with a measured
distribution. Judgement-based range: if the device increments (I2, I3) and the research-heavy increments (I5, I7)
each need one extra timebox because a Must overruns beyond its timebox contingency, the duration becomes 46 weeks;
if G4 stops the programme, the project ends at Week 20 (10 timeboxes, 92.0 ideal days planned). Comparison with the
review's own P0 budget (one engineer, two weeks of CPU training plus a CUDA residency check, HVX port in week three;
review l564-565): that budget covers a research prototype; this plan also builds the assertion harness, the fp64
reference library, the production CUDA and adapter paths and the evidence package before the formal decision. The
comparable milestone is the G1 early read at Week 8 (six weeks after Foundations), which is where this plan is
three to four weeks slower than the review's budget, by design.


## 11. Management approach

### 11.1 Facilitated workshops

| Workshop | When | Participants (vacancies noted) | Inputs | Outputs / decision |
|---|---|---|---|---|
| Terms of Reference | Week 1, day 1 (2 h) | Engineer; Business Sponsor (VACANT: invite the recommended budget holder) | review Secs. 1, 7; bridge Sec. 0 | ToR (interim or signed) |
| Feasibility | Week 1, days 3-4 (2 h) | Engineer; Technical Advisor (VACANT) if available | Feasibility desk check; hardware inventory | Feasibility Assessment; proceed / stop |
| Foundations | Week 2, days 1-2 (half day) | Engineer; Workshop Facilitator and DSDM Coach (VACANT: recommended external); Sponsor / Visionary (VACANT) | Draft PRL, SAD, DAD, MAD, Delivery Plan | Baselined PRL with MoSCoW; F4 "matches" definition; working-set measurement protocol (PR-14); gate criteria; G0 |
| Timebox kick-off | Day 1 of every timebox (1 h) | Engineer; agent sessions prepared with their PRL ids | Timebox plan (Sec. 5.5) | Confirmed scope and agent assignment |
| Timebox review and demo | Day 10 of every timebox (1 h) | Engineer; Business Ambassador (VACANT: recorded as "not reviewed by business") | Evidence logs | Timebox Review Record; MoSCoW renegotiation |
| Retrospective | Day 10 of every timebox (15 min) | Engineer | Stand-up log | One process change, recorded |
| Split-design review | TB3 (2 h) | Engineer; Business Advisor (VACANT: recommended evaluation advisor) | P0-01, P0-03 | Leakage check signed |
| G1 early read | End of Week 8 (1 h) | Engineer; Sponsor (VACANT) | P0-05, P0-06 results | G1 decision |
| G4 P0 decision | End of Week 18 (2 h) | Engineer; Sponsor, Visionary, Ambassador, Advisor (all VACANT: G4 needs at least one external signature); Facilitator (recommended) | P0 metrics table (P0-17) | G4 decision record |
| Increment review | End of I2, I3, I5, I6, I7 (1 h) | Engineer; Visionary (VACANT) | Project Review Report (increment) | Continue / re-plan |
| Final project review | Weeks 37-38 (2 h) | Engineer; Sponsor (VACANT) | Final Project Review Report | Close; Benefits Assessment plan |

Self-facilitation checklist (used whenever the Facilitator role is vacant): state the objective and the decision
rule before the meeting; list the inputs (with document citations); take the decision by the rule, not by
discussion; record the outcome, the dissent (if any) and the owner of each action.

### 11.2 Daily stand-up for a solo engineer with agents

A written log entry at the start of each working day, about 10 minutes, one line per heading:

```
DATE / TB / day n of 10
DONE:     PRL ids moved; tests turned green (id: measured vs expected)
TODAY:    PRL ids; which agent session (developer / tester) gets which id
BLOCKED:  hardware, toolchain, document question (open as a bridge R9 conflict record), usage limit
AGENTS:   sessions running; outputs awaiting review; usage consumed vs timebox budget
MOSCOW:   any Must at risk? if yes, which Could / Should is dropped today
```

Agents append their own entries in the same format for the PRL ids they hold. The log is the input to the
Timebox Review Record and the status report.

### 11.3 MoSCoW renegotiation at timebox review

  1. Musts change only with a recorded Sponsor decision (interim while vacant) and a reason tied to a document
     section (e.g. a new erratum); never to fit the date.
  2. An unfinished Should moves to the next timebox as a Should or is demoted to Could; an unfinished Could is
     dropped or re-queued as a Could.
  3. After carry-over, recompute the 60 % rule for the next timebox (awk check, Sec. 4.3); if it is broken,
     demote a Should or move a Must to a later timebox only when its dependency chain and the gate allow it.
  4. A new requirement enters the PRL only with a document citation and an acceptance criterion; otherwise it
     is recorded as a gap (bridge Sec. 11 format) and not scheduled.
  5. Agents may propose changes in their log; the engineer decides and records the reason.

### 11.4 Escalation

| Level | Tolerance held by | Within tolerance | Escalate when |
|---|---|---|---|
| Timebox | Team Leader (interim: engineer) | Drop or carry Shoulds / Coulds; re-assign agents | A Must is at risk on day 6; CI-HASH fails; agent budget exhausted before day 5 |
| Increment | Project Manager (interim: engineer), informing the Sponsor | Move a Must within the increment; delay a gate by at most one timebox | A gate fails; an increment needs a second extra timebox |
| Programme | Business Sponsor (VACANT: interim record, ratified later) | Gate outcomes; stop / redirect; scope changes; adding increments | Any G4 outcome other than PASS; any change to the Won't list |
| Technical conflict | Technical Coordinator | Resolve by bridge R1-R13 | A conflict R1-R13 cannot resolve: bridge R9 record plus erratum proposal (bridge 8.8) |

### 11.5 Reporting: one-page status template (issued at every close-out)

```
QNLP programme status  --  TB<n> (Weeks a-b), increment I<k>          Evidence labels per claim
1. Overall: GREEN / AMBER / RED  (one sentence why)
2. Gate: next gate G<m> at Week <w>; criteria on track? (yes/no per criterion)
3. PRL this timebox: Musts done x/y (ideal days a/b); Shoulds x/y; Coulds x/y
4. Tests turned green (id: measured vs expected); tests failing (id, value)
5. Velocity: ideal days delivered this timebox; running mean; vs 10-day assumption
6. Agent usage: budget vs consumed; items lost to limits
7. Risks: top 3 (id, P x I, change since last report); risks retired (id, closing test)
8. UNVERIFIED items closed (bridge Sec. 10 letter, dated note); gaps raised (G-id)
9. Errata / addendum proposals (ids, status)
10. Decisions needed from the Sponsor (VACANT: interim decisions listed for ratification)
11. Next timebox: objective and Must ids
```


## 12. Benefits

### 12.1 Benefits the Business Case claims (all falsifiable)

| Id | Benefit | Source | Measure | Target / falsifier | When |
|---|---|---|---|---|---|
| BN-1 | Order-sensitive compositional generalisation (role swap) | review l561-562 | P0 role-swap accuracy, 5 seeds | QNLP >= 90 %; B1 = 50 %; B2 55-80 % expected; falsified if QNLP <= B2 at 4K sentences (F1) | G4 (Week 18) |
| BN-2 | In-distribution accuracy | review l561-562 | P0 IID accuracy | QNLP >= 97 % expected; falsified if < 90 % (F2) | G4 |
| BN-3 | Generalisation to unseen (S, V, O) triples | review l562 | Triple-holdout accuracy | QNLP >= 90 %; B2 70-90 % expected | G4 |
| BN-4 | Almost-zero working set | review l563; errata E-4 (c) | Measured state bytes per sentence (QC-14, CUD-02) | <= 128 B (Model A) / <= 416 B (Model B); falsified if exceeded (F3); materialised objects 0 B | G3 / G4 |
| BN-5 | Small persistent model (a weight table, not zero) | review l551-558; errata E-4 (a) | Model bytes | Model A 352 B, Model B 640 B vs B1 528 B, B2 46.2 KB, B3 28.6 KB | G4 |
| BN-6 | The quantum parameterisation adds value at equal bytes | review l558-559, l563; companion c450 | QNLP vs equal-byte direct-factor model per split | Falsified if the direct-factor model matches QNLP on every split (F4) | G4 |
| BN-7 | Exact Born probabilities with a computable error certificate on the NPU | errata E-11; spec l1258; errata l87 | Per-sentence certificate; Q1.15 vs fp64 | Every P_c within 2e/(2 sqrt D - e) or routed to fp32 at D < 2^-6; T6(b) within 2e-3 (worked example measured on host 3.11e-5) | G3 |
| BN-8 | Deterministic, data-independent op count | addendum 15.3(a) l1560-1563 | Counted ops per sentence (P0-OPS) | Constant per shape; ~1.5-2.7 K FMA-class per sentence at (1,3) (addendum estimate) | G4 |
| BN-9 | Train on GPU, infer on NPU with no weight or state traffic after load | spec 3.2 l675-676; spec 6.3 l1138-1139; errata E-5 | Traffic per sentence (QC-13) | Word ids in; 2^{q_s} + 1 outputs out (12 B at (1,1), spec l1166) | G3 |
| BN-10 | Wider grammar at the cost of the verb | addendum Sec. 17 l1744-1789 | Peak amplitudes (T22, GRM-11, GRM-13) | Relative clause 14 / 44 amplitudes at (1,1) / (2,1) (10 / 36 in place) | G5 |
| BN-11 | Coverage of free paraphrases | addendum 15.4 item 5 | Parse-FAIL rate (GRM-19) | 10-30 % expected (a hypothesis, reported either way) | G5 |
| BN-12 | Productivity on longer texts | addendum 15.4 items 2(iii), 5 | Length split accuracy (DSC-14) | QNLP holds where shapes are covered (a hypothesis) | G7 |
| BN-13 | An early, cheap negative answer if the premise fails | review Sec. 7; companion 16.3 | Week of the G4 decision; effort spent | Decision by Week 18 and closure by Week 20 after at most 92.0 planned ideal days, instead of the full 160.5 | G4 |

### 12.2 How each benefit is measured

  - BN-1..BN-3, BN-5, BN-6: the P0 metrics table (P0-17) over 5 seeds with the uncertainty of P0-19; baselines
    trained with the same data and budget (P0-16); figures reported at each model's own bytes (addendum 15.4
    item 4).
  - BN-4: the working-set protocol fixed at G0 (risk PR-14): compiler register / local-memory report plus declared
    per-sentence scratch, on CUDA (PTXAS) and HVX (HVX-REG, QC-14).
  - BN-7: per-sentence certificate emitted by the readout (M7); fallback rate reported.
  - BN-8, BN-9: op counter (P0-OPS) and traffic accounting in the end-to-end harness (QC-13).
  - BN-10..BN-12: the tests and experiments named, in I5 and I7.
  - BN-13: the status reports' effort and dates.

### 12.3 Benefits Enablement (at each Deployment)

Each increment's Deployment records which benefits it enables and their measured values so far: I1 (BN-7 on the
host), I2 (BN-4 on CUDA), I3 (BN-4, BN-7, BN-9 on the NPU), I4 (BN-1..BN-6, BN-8, BN-13), I5 (BN-10, BN-11), I7
(BN-12). Enablement means the evidence package reproduces the value (REPRO), not that a user has adopted it.

### 12.4 Benefits Assessment plan

| Assessment | When | Owner | Method | Output |
|---|---|---|---|---|
| Interim | Week 20 (after G4) | Business Visionary (VACANT; interim engineer, with external reviewer recommended) | Compare BN-1..BN-9 and BN-13 with their targets; apply the companion's advancement gates (companion 13.5 items 1-6: structural and semantic behaviour independently checked; invariances and numeric policies hold; resource measurements cover the full boundary including fallback; held-out quality meets the predeclared criterion with uncertainty; no silent budget overrun; negative results included) | Interim Benefits Assessment; input to G4 ratification |
| Final | 12 weeks after the last Deployment (Week 50 nominal), or 12 weeks after a STOP at G4 | Business Visionary (VACANT) | Same method over all benefits delivered; note any external use of the releases | Benefits Assessment product |

A falsified P0 is assessed as a delivered benefit of type BN-13 (an early negative answer), with the negative-result
record (DEP-03) as its evidence.


## Appendix A. Plan-level gap raised with the bridge (G31) and decisions to ratify

G31 (registered in bridge Sec. 11 by this plan): no document specifies the HVX layout or residency for (1, q_s = 3),
the P0 Model B configuration (review l552-553; addendum 15.3(a) l1553-1558). spec 3.2 l709-718 gives
sentence-per-lane for (1,1) and amplitude-per-lane for (2,1) only; the review verifies HVX residency only for the
(1,1) lane layout (review l553-554). Closing action: QC-12 layout spike and measurement in TB7; the result is
proposed to the addendum owner as a Sec. 12.3 / Sec. 16 row.

Decisions this plan needs ratified at G0 (they are procedural, not technical, and are fixed before any P0 run):
  1. Operational definition of "matches" in falsifier F4 (proposal in Sec. 5.4).
  2. Working-set measurement protocol for falsifier F3 (risk PR-14).
  3. The G4 decision table (Sec. 5.4), including STOP as the default outcome of F1 / F2.

End of plan.
