# TOPICS: TF-A qcom/integration (public platforms)

Base: TF-A integration 0752f3bf2808.
Every qcom/<platform> branch below is the pinned base plus
the LATEST revision of every in-flight upstream submission for that
platform (Gerrit patch set, GitHub PR head, lore series version) and our
not yet submitted patches unchanged (refresh 2026-10-25). This branch
merges every platform branch onto the base; each merge commit message
records its conflict resolutions, the ones redone in the refresh under
"Conflicts (refresh 2026-10-25)". Snapshot of the gated state before the
refresh: tag qcom/integration-20261024.

Added 2026-10-26: qcom/kailua (SM8550 HDK), merged on top of the gated
refresh; the state before it is tag qcom/integration-20261025.

Added 2026-10-31: qcom/eliza (SM7750 MTP), merged on top of the
2026-10-26 state (tag qcom/integration-20261026).

## Platform branches

| platform | branch | on | carries | upstream status |
|---|---|---|---|---|
| kodiak | qcom/kodiak | base | xPU syndrome fix; Gerrit topic qti-kodiak-native (55532..55541 PS2, 55670/55671 PS1) | open; xPU fix not submitted |
| monaco | qcom/monaco | base | Gerrit topic qti-monaco-v2 (55528/55529/55531 PS2, 55530 PS3, 54579 PS7 Dinesh, 55765..55767 PS1); 55253 PS4; 55088 PS3 | open; 55766 and 55088 carried adapted (shikra define, base without the abandoned CNTFRQ series) |
| lemans | qcom/lemans | kodiak | the monaco set; 53616 PS4 (Dinesh, PDC hooks); 54830 PS1 (Dinesh, SYSTEM_RESET2) + two fixes; GPU SMMU aperture | open; aperture and the two SYSTEM_RESET2 fixes not submitted |
| talos | qcom/talos | kodiak | Talos SoC, PDC, XBL boot, PON; 54926 PS8; 55530 PS3 | Talos patches not submitted |
| lanai | qcom/lanai | kodiak | SM8650 QRD | not submitted |
| kailua | qcom/kailua | lanai | SM8550 HDK (shares the Lanai DSU power code) | not submitted |
| agatti | qcom/agatti | base | Igor's topic agatti-uno-q-reworked (55728..55735 PS1) + access control, TBB, aperture, PSCI fixes | topic open; fixes not submitted |
| shikra | qcom/shikra | base | 53770 PS9 (IPCC, Nord and Shikra); shikra-enable topic merged in the base | open |
| eliza | qcom/eliza | lanai | SM7750 MTP: platform, PDC, watchdogs, secondary cores, PSCI State ID decode and BL31 suspend hold, SMMUs, LPASS IMEM, RPMh TZ client, GPU GX power-up; 55530 PS3 | not submitted; 55530 open |

## Per-patch submission map

Each commit of a platform branch (from its parent: the base or the branch it builds on) and the upstream submission it is the latest revision of. "adapted": the patch differs from the posted revision because the pinned base or another series changed under it; the merge commit message or the commit itself says how. "not submitted": local work, carried unchanged.

### qcom/monaco (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 1151bd08cf81 | refactor(qti): share the RPMh data between Lemans and Monaco | Tyler Baker | Gerrit 55528 PS2 (ee6488e0d9, open) |  |
| 42c9212cb1fb | feat(monza): add Monaco clock back-end | Tyler Baker | Gerrit 55529 PS2 (403c07f768, open) |  |
| 0fee066759d5 | fix(qti): only advertise SYSTEM_SUSPEND with a system-level state | Tyler Baker | Gerrit 55530 PS3 (e93680bd7f, open) |  |
| 266b60a31287 | fix(qti): program the Monaco GPU SMMU aperture | Tyler Baker | Gerrit 55531 PS2 (c76d83df73, open) |  |
| b21f5efe1a4a | feat(qti): notify SAIL on reset and shutdown | Dinesh Choudhary | Gerrit 54579 PS7 (5976d0bcf7, open) |  |
| ee2467075c85 | feat(monza): notify SAIL on reset and shutdown | Tyler Baker | Gerrit 55765 PS1 (3d00cef693, open) |  |
| 5f11b9381a3c | refactor(qti): take the SYSTEM_RESET reset type from the platform | Tyler Baker | Gerrit 55766 PS1 (24b95045ab, open) | adapted |
| 6eea0cb7086d | fix(monza): use a PMIC hard reset for SYSTEM_RESET | Tyler Baker | Gerrit 55767 PS1 (25e18b7d09, open) |  |
| 466fd4a19b5f | refactor(platforms): leave CNTFRQ_EL0 to PSCI | Tyler Baker | Gerrit 55253 PS4 (2c5e6debe8, open) |  |
| e7e26831cdee | fix(psci): run the arch setup before pwr_domain_on_finish() | Tyler Baker | Gerrit 55088 PS3 (59eb165ec8, open) | adapted |

### qcom/agatti (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| ed6fcae6c8d3 | feat(cpus): let a derived core override the Cortex-A53 MIDR | Igor Opaniuk | Gerrit 55728 PS1 (4ae853916f, open) |  |
| 60919e323be3 | feat(qti): add Kryo 2xx Silver CPU driver | Igor Opaniuk | Gerrit 55729 PS1 (6eb93d2094, open) |  |
| 46994a91f665 | refactor(qti): make the core power-down request a PM backend hook | Igor Opaniuk | Gerrit 55730 PS1 (51211142f3, open) |  |
| 0842110e2879 | refactor(qti): make the secure error interrupts optional in the GIC table | Igor Opaniuk | Gerrit 55731 PS1 (5926cd6844, open) |  |
| 429abca97ab1 | refactor(qti): take the SPMI arbiter base from the SoC definitions | Tyler Baker | Gerrit 55732 PS1 (8650aff046, open) |  |
| 5ec0e0782d5c | feat(qti): add Agatti SoC support | Tyler Baker | Gerrit 55733 PS1 (6e3bc97471, open) |  |
| 53369b78648a | feat(uno_q): add support for Arduino UNO Q | Tyler Baker | Gerrit 55734 PS1 (bfbb8c154f, open) |  |
| d9240af106d4 | docs(qti): add Arduino UNO Q documentation | Tyler Baker | Gerrit 55735 PS1 (8e11c46948, open) |  |
| 1489149e6b2c | fix(qti): read the xPU syndrome registers at their offsets | Tyler Baker | not submitted | (local Change-Id) |
| 39283cd5eab5 | fix(qti): write the upper partition address only on wide MPUs | Tyler Baker | not submitted | (local Change-Id) |
| 207f44517e32 | feat(qti): add Agatti access control configuration | Tyler Baker | not submitted | (local Change-Id) |
| 69e8d5c63095 | feat(uno_q): protect BL31 and BL32 from the normal world | Tyler Baker | not submitted | (local Change-Id) |
| 6e06ee51ca3c | feat(qti): let BL2 lock memory before it reads the FIP | Tyler Baker | not submitted | (local Change-Id) |
| cef8af1114b1 | fix(uno_q): lock the FIP staging window before BL2 reads it | Tyler Baker | not submitted | (local Change-Id) |
| 8477b428d961 | feat(qti): let BL2 authenticate the images it loads | Tyler Baker | not submitted | (local Change-Id) |
| 7a9de0fc1913 | feat(uno_q): support trusted board boot | Tyler Baker | not submitted | (local Change-Id) |
| bc7beb1ef7b8 | feat(qti): program the Adreno SMMU aperture on Agatti | Tyler Baker | not submitted | (local Change-Id) |
| e839f0e6f294 | feat(qti): support a separate BL2 NOLOAD region | Tyler Baker | not submitted | (local Change-Id) |
| 2b23c5ebce7a | fix(uno_q): keep the TBB BL2 in IMEM | Tyler Baker | not submitted | (local Change-Id) |
| 7f4a15b09c26 | fix(psci): offer CPU_SUSPEND to platforms with only CPU standby | Tyler Baker | not submitted | (local Change-Id) |

### qcom/kodiak (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 9bebaf3952a1 | fix(qti): read the xPU syndrome registers at their offsets | Tyler Baker | not submitted | (local Change-Id) |
| 850d5c9b0b16 | fix(qti): drop the gold cluster settle delay | Tyler Baker | Gerrit 55670 PS1 (0ef9e67533, open) |  |
| 34983b2b015a | refactor(qti): parameterize gold domain setup | Tyler Baker | Gerrit 55532 PS2 (78fd34ad8e, open) |  |
| dd97d999d0da | fix(qti): fail CPU_ON when the gold domain does not come up | Tyler Baker | Gerrit 55533 PS2 (e7be43fbc5, open) |  |
| d41924a97757 | fix(qti): stop the boot when CPUCP cannot be started | Tyler Baker | Gerrit 55534 PS2 (df78b8455c, open) |  |
| ece32f35aba6 | feat(qti): add a CPU_OFF holding pen | Tyler Baker | Gerrit 55535 PS2 (e16d57de41, open) |  |
| 4b2746084bfa | feat(qti): initialize the Kodiak gold domain | Tyler Baker | Gerrit 55536 PS2 (46fbe596d8, open) |  |
| 2ae8c0b1c173 | feat(qti): add Kodiak CPUCP support | Tyler Baker | Gerrit 55537 PS2 (e26b093311, open) |  |
| 45083855e5b5 | feat(qti): add the Kodiak PDC sequences | Tyler Baker | Gerrit 55538 PS2 (2ce7e92c10, open) |  |
| dbe3ba1b517f | feat(qti): add the SiP GPU SMMU aperture call | Tyler Baker | Gerrit 55671 PS1 (5293cb8ab6, open) |  |
| 1dba886c7354 | feat(qti): initialize the Kodiak GPU SMMU | Tyler Baker | Gerrit 55539 PS2 (d2ee4b8224, open) |  |
| 2cf645920335 | feat(rb3gen2): boot without QTISECLIB | Tyler Baker | Gerrit 55540 PS2 (46203adead, open) | adapted |
| 638a8448d3a6 | docs(rb3gen2): describe the native boot | Tyler Baker | Gerrit 55541 PS2 (e0a2479fac, open) |  |

### qcom/lemans (on qcom/kodiak)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 2b34e368e771 | refactor(qti): share the RPMh data between Lemans and Monaco | Tyler Baker | Gerrit 55528 PS2 (ee6488e0d9, open) |  |
| b4f1e0d0e215 | feat(monza): add Monaco clock back-end | Tyler Baker | Gerrit 55529 PS2 (403c07f768, open) |  |
| 274dbf1277ac | fix(qti): only advertise SYSTEM_SUSPEND with a system-level state | Tyler Baker | Gerrit 55530 PS3 (e93680bd7f, open) |  |
| 5a0845d106cd | fix(qti): program the Monaco GPU SMMU aperture | Tyler Baker | Gerrit 55531 PS2 (c76d83df73, open) | adapted |
| 33e62b9f3180 | feat(qti): notify SAIL on reset and shutdown | Dinesh Choudhary | Gerrit 54579 PS7 (5976d0bcf7, open) |  |
| 5a90d00ccfe7 | feat(monza): notify SAIL on reset and shutdown | Tyler Baker | Gerrit 55765 PS1 (3d00cef693, open) |  |
| 2f5df2d8fee8 | refactor(qti): take the SYSTEM_RESET reset type from the platform | Tyler Baker | Gerrit 55766 PS1 (24b95045ab, open) | adapted |
| d1989862d452 | fix(monza): use a PMIC hard reset for SYSTEM_RESET | Tyler Baker | Gerrit 55767 PS1 (25e18b7d09, open) |  |
| 11fcaf2613ee | refactor(platforms): leave CNTFRQ_EL0 to PSCI | Tyler Baker | Gerrit 55253 PS4 (2c5e6debe8, open) |  |
| 1e8af1cab531 | fix(psci): run the arch setup before pwr_domain_on_finish() | Tyler Baker | Gerrit 55088 PS3 (59eb165ec8, open) | adapted |
| 84a2d6be9183 | feat(qti): add PDC chipset hooks for LeMans | Dinesh Choudhary | Gerrit 53616 PS4 (96e5bf201e, open) | adapted |
| f52bac0b21bb | feat(qti): add PSCI SYSTEM_RESET2 support | Dinesh Choudhary | Gerrit 54830 PS1 (eb452efd5e, open) | adapted |
| d29fde3c41d1 | fix(qti): program the Lemans GPU SMMU aperture | Tyler Baker | not submitted | (local Change-Id) |
| 02a8e694affc | fix(qti): program the PMIC reset type that SYSTEM_RESET2 asks for | Tyler Baker | not submitted | (local Change-Id) |
| 657d88241416 | fix(qti): offer SYSTEM_RESET2 only with the reset cookie store | Tyler Baker | not submitted | (local Change-Id) |

### qcom/talos (on qcom/kodiak)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 47e0cde04c9c | feat(qti): add Talos SoC definitions | Tyler Baker | not submitted | (local Change-Id) |
| 7274cb3b6f40 | refactor(qti): move qcs615 under the talos directory | Tyler Baker | not submitted | (local Change-Id) |
| 88ca30aaf08d | feat(qti): add Talos apps PDC tables | Tyler Baker | not submitted | (local Change-Id) |
| c816f651c433 | fix(qti): reject MEM_ASSIGN without an access control backend | Tyler Baker | not submitted | (local Change-Id) |
| e0e269ff0408 | feat(qcs615_ride): add XBL boot support | Tyler Baker | not submitted | (local Change-Id) |
| 3b6a0af29dea | docs(qti): describe the Talos boot flow | Tyler Baker | not submitted | (local Change-Id) |
| be19b62556c6 | fix(psci): allow OS-initiated mode while other cores are off | Tyler Baker | Gerrit 54926 PS8 (0307c3044d, open) |  |
| a257b6104ffe | fix(qti): only advertise SYSTEM_SUSPEND with a system-level state | Tyler Baker | Gerrit 55530 PS3 (e93680bd7f, open) |  |
| b3fbecd10ddd | fix(qti): route the Talos PON interrupts to the apps | Tyler Baker | not submitted | (local Change-Id) |

### qcom/lanai (on qcom/kodiak)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| 15713c960e52 | fix(qti): spmi_arb: take the arbiter layout from the platform | Tyler Baker | not submitted |  |
| 4de9db4ae03a | fix(qti): spmi_arb: use the APID owned by the secure world | Tyler Baker | not submitted |  |
| becaa28e83b4 | fix(qti): let the platform choose the PS_HOLD reset type | Tyler Baker | not submitted |  |
| 4668cfb77043 | refactor(qti): take the hoya GIC base from the chipset | Tyler Baker | not submitted |  |
| 14b120f0023a | feat(qti): add the SM8650 (Lanai) QRD platform | Tyler Baker | not submitted |  |
| 212ca12b538b | feat(qti): pdc: support PDCs whose sequences the AOP loads | Tyler Baker | not submitted |  |
| 0792c3ed398d | feat(qti): lanai: enable the apps PDC | Tyler Baker | not submitted |  |
| 3a47fdda98a1 | feat(qti): lanai: power on the secondary cores | Tyler Baker | not submitted |  |
| ce6dd686f515 | feat(qti): watchdog: support a write-only control register | Tyler Baker | not submitted |  |
| 6f5cdaf17587 | feat(qti): lanai: service the secure watchdog | Tyler Baker | not submitted |  |

### qcom/shikra (on the base)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| dc6def90397f | feat(qti): add IPCC driver with Nord and Shikra support | pradeep-kondoju | Gerrit 53770 PS9 (ded6567822, open) |  |


### qcom/kailua (on qcom/lanai)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| a3ec68aed521 | refactor(qti): hoya: share the DSU core power-up between chipsets | Tyler Baker | not submitted |  |
| b203ab8c5ee1 | feat(qti): hoya: power up silver complexes before their cores | Tyler Baker | not submitted |  |
| ad8fc4429740 | feat(qti): add the SM8550 (Kailua) HDK platform | Tyler Baker | not submitted |  |
| 7b67e893061f | feat(qti): kailua: enable the apps PDC | Tyler Baker | not submitted |  |

### qcom/eliza (on qcom/lanai)

| commit | subject | author | submission | note |
|---|---|---|---|---|
| e69326236519 | fix(qti): only advertise SYSTEM_SUSPEND with a system-level state | Tyler Baker | Gerrit 55530 PS3 | same patch as on qcom/talos |
| 0a2e3577dac3 | feat(qti): add the SM7750 (Eliza) MTP platform | Tyler Baker | not submitted |  |
| 2fd2d83f97a8 | feat(qti): eliza: give the apps PDC inputs to the HLOS DRV | Tyler Baker | not submitted |  |
| ae89c371f7e7 | feat(qti): eliza: service the secure watchdog | Tyler Baker | not submitted |  |
| ce8db72873f1 | feat(qti): eliza: power on the secondary cores | Tyler Baker | not submitted |  |
| 19732bbaea6e | feat(qti): eliza: give the non-secure watchdog to the normal world | Tyler Baker | not submitted |  |
| d7a821d64046 | refactor(qti): let the platform decode the PSCI State ID | Tyler Baker | not submitted |  |
| 974904d2d9cf | feat(qti): hoya: hold cores suspended to a power-down state in BL31 | Tyler Baker | not submitted |  |
| 98407a35e8d2 | feat(qti): eliza: accept the device tree idle states | Tyler Baker | not submitted |  |
| fe14225c6bd2 | feat(qti): eliza: release the LPASS boot IMEM at cold boot | Tyler Baker | not submitted |  |
| 3ff4a8f5e69b | feat(qti): smmu: enable the GPU SMMU aperture where it has a control | Tyler Baker | not submitted |  |
| e55c608b03d9 | feat(qti): eliza: set up the apps and GPU SMMUs | Tyler Baker | not submitted |  |
| 07f204f4228a | docs(qti): eliza: note that SYSTEM_RESET does not reset the MTP | Tyler Baker | not submitted |  |
| a0b8ad371869 | feat(qti): eliza: hand the GPU GDSC controls to the voting logic | Tyler Baker | not submitted |  |
| c3ca2588d6aa | feat(qti): add a platform hook for clocked cold-boot setup | Tyler Baker | not submitted |  |
| e33d5b7dfafd | feat(qti): eliza: enable the RPMh TZ client | Tyler Baker | not submitted |  |
| 00d12c5b13c0 | feat(qti): eliza: run the GPU GX power-up sequence at cold boot | Tyler Baker | not submitted |  |

## In-flight submissions not carried

- 55272 PS1 shikra-accesscontrol (layered XPU3/VMIDMT restructure; no Shikra code yet): carrying it means porting the unsubmitted Agatti access control (BL2 lock, dynamic locks, wide MPU) and the xPU syndrome fix onto it; held for Tyler.
- 55206 (shikra-enable): a tf-a-ci-scripts build config, not this tree.
- Generic changes not tied to a platform: 53290 GICv3 routing class, 51930 PSCI null checks, 52985 QTI to QCOM rename, 55491 maintainers.

## LAVA results

Build: one fresh firmware fingerprint per build (QCDFP-<date>-<n>), read
back from BL2, BL31, OP-TEE and U-Boot on the cold boot and on the second
boot where the board comes back (Lemans: SPL instead of BL2, Shikra: U-Boot
only). Suite: the OSS suite minus ACS, compared case by case with the
earlier integration gate and the platform baseline. Every job ends on the
lab stock firmware (in-job restore block, or a pinned restore job where the
job cannot reach it). Jobs: https://lava.infra.foundries.io/scheduler/job/<id>.

| platform | stack | build | job | result vs baseline |
|---|---|---|---|---|
| kodiak | public | QCDFP-261025-83 | 512685 | equal to 502713 except policy7-min fail -> pass |
| agatti | public | QCDFP-261025-86 | 512743 | equal to 511900 and 490883 |
| monaco | public | QCDFP-261025-89 | 512751 | equal to 511893; same warm reboot gap; backstop 512754 |
| lemans | public | QCDFP-261025-84 | 512687 | equal to 511912 except the same flake |
| talos | public | QCDFP-261025-92 | 512759 | equal to 511901 except the known KVM PMU timing race (now passing) and the OSM cpufreq checks (not carried, skip) |
| shikra | public | QCDFP-261025-91 | 512770 | U-Boot only (Shikra v4) on the stock XBL/QSEE/Gunyah health image: FP, Linux, login and the health smoke tests pass as health job 511579; no upstream TF-A/OP-TEE boot path yet |
| lanai | public |  |  | untested: no LAVA board |
| kailua | public |  |  | untested: no board yet (lab access pending) |
| eliza | public | QCDFP-261031-408 | MTP, no LAVA board | BOARD-REQUEST-3: TF-A/OP-TEE/U-Boot FP, U-Boot UFS clock lines gone, UFS, fastboot, GPU, NPU, CPU hotplug pass (BOARD-REQUEST-1 on 401 passed the same steps) |
