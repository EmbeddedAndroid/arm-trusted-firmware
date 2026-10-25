# TOPICS: TF-A qcom/integration (public platforms)

Base: integration 0752f3bf2808. This branch merges every qcom/<platform>
branch below onto the base (a branch that another one builds on comes in
through that branch's merge); each merge commit message records its
conflict resolutions and adaptations.

| platform | branch | depends on | series | upstream status |
|---|---|---|---|---|
| kodiak | qcom/kodiak | base | native BL31 + xPU syndrome fix | Gerrit topic qti-kodiak-native; xPU fix local |
| monaco | qcom/monaco | base | Monaco clock back-end, SAIL notify, GPU aperture, reset type | Gerrit topic qti-monaco-v2; the reset-type change needs a new patch set (shikra define) |
| lemans | qcom/lemans | kodiak | copy of the monaco series + Lemans GPU SMMU aperture | as monaco; aperture local |
| talos | qcom/talos | kodiak | Talos SoC, PDC, boot, PON | local; PSCI OSI on Gerrit |
| lanai | qcom/lanai | kodiak | SM8650 QRD | local |
| agatti | qcom/agatti | base | Igor's agatti-uno-q-reworked topic + access control, TBB, aperture and PSCI fixes | Igor's topic on Gerrit; fixes local |

The LAVA builds used TF-A f2b3d43e0, OP-TEE qcom/integration
bd6f523e58b4 and U-Boot qcom/integration 82fbe4d77385.

## LAVA results

Build: one fresh firmware fingerprint per build (QCDFP-<date>-<n>), read
back from BL2, BL31, OP-TEE and U-Boot on the cold boot and, where the
board comes back, the second boot (Lemans: SPL instead of BL2). Suite:
the OSS suite minus ACS, compared case by case with the platform
baseline. Every board ends on stock firmware; Monaco through a separate
restore job, because its warm reboot hangs. Jobs:
https://lava.infra.foundries.io/scheduler/job/<id>.

| platform | build | job | result vs baseline |
|---|---|---|---|
| kodiak | QCDFP-261024-69 | 511916 | equal to 502713 except the big-core cpufreq runtime-ratio check, a board flake also seen with other firmware |
| agatti | QCDFP-261024-68 | 511900 | equal to 490883 |
| monaco | QCDFP-261024-67 | 511893 | equal to 502190 (gpu-tflite x2 now pass) up to the warm reboot, which hangs (SAIL acknowledge, open) |
| talos | QCDFP-261024-62 | 511901 | as 492393 except: no OSM cpufreq (not carried); kvm-unit-tests pmu-overflow-interrupt, a KVM PMU timing race that fails as often with the previous firmware (per run 2.2 % vs 3.1 %) |
| lemans | QCDFP-261024-61 | 511912 | no regression vs 509672 (cpufreq and KVM PMU now pass); U-Boot SPL not gated |
| lanai | | | untested: no LAVA board |
