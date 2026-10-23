/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_internal.h>
#include <lib/utils_def.h>

/* PDC interrupt mapping for SM8650 APSS: the HLOS DRV owns every input. */
struct pdc_interrupt_mapping g_pdc_interrupt_mapping[] = {
	/* Bit 0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 512 }, /* spmi_protocol_irq[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 513 }, /* ee0_apps_hlos_spmi_periph_irq_0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 514 }, /* ee1_apps_trustzone_spmi_periph_irq_0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 515 }, /* ee0_apps_hlos_spmi_periph_irq_1 */

	/* Bit 4 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 516 }, /* ee1_apps_trustzone_spmi_periph_irq_1 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 517 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 518 }, /* wdog_exp_irq_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 519 }, /* lpass_sdc_gpo_bit0 */

	/* Bit 8 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 520 }, /* xpu_irq_summary_apps */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 521 }, /* vmidmt_irq_summary_apps */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 522 }, /* o_i3c_ibi_irq[26] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 523 }, /* u_eud_eud_int_mx[1] */

	/* Bit 12 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 524 }, /* lpi_lb_interrupt[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 525 }, /* lpi_lb_interrupt[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 526 }, /* u_eud_eud_p0_dpse_int_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 527 }, /* u_eud_eud_p0_dmse_int_mx */

	/* Bit 16 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 528 }, /* codec_tx_swr_irq[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 529 }, /* u_cm_usb3_dp_con_qmp_usb3_lfps_rxterm_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 530 }, /* aoss_all_xpu_malicious_summ_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 531 },

	/* Bit 20 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 532 }, /* tsens0_tsens_0C_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 533 }, /* Spare */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 534 }, /* pdc_apps_epcb_timeout_summary_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 535 }, /* spmi_protocol_irq[0] */

	/* Bit 24 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 536 }, /* o_i3c_ibi_irq[6] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 537 }, /* o_i3c_ibi_irq[10] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 538 }, /* tsense0_upper_lower_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 539 }, /* tsense1_upper_lower_intr */

	/* Bit 28 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 540 }, /* tsense2_upper_lower_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 541 }, /* Spare */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 542 }, /* i3c_ibi_irq_s0[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 543 }, /* i3c_ibi_irq_s0[1] */

	/* Bit 32 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 544 }, /* i3c_ibi_irq_s1[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 545 }, /* i3c_ibi_irq_s1[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 546 }, /* i3c_ibi_irq_s2[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 547 }, /* i3c_ibi_irq_s2[1] */

	/* Bit 36 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 548 }, /* i3c_ibi_irq_s3[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 549 }, /* i3c_ibi_irq_s3[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 550 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 551 },

	/* Bit 40 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 552 }, /* bi_mx_lpass_1_aoss_mx_gpio_166 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 553 }, /* o_i3c_ibi_irq[2] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 554 }, /* o_i3c_ibi_irq[14] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 555 }, /* o_i3c_ibi_irq[18] */

	/* Bit 44 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 556 }, /* o_i3c_ibi_irq[22] */
	/* lpi_lpiaon_noc_sidebandManagerfault_summary */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 557 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 558 }, /* Spare */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 559 }, /* i3c_ibi_irq_s4[0] */

	/* Bit 48 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 560 }, /* i3c_ibi_irq_s4[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 561 }, /* i3c_ibi_irq_s5[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 562 }, /* i3c_ibi_irq_s5[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 563 }, /* aoss_irq_core_in_mx_gpio_59 */

	/* Bit 52 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 564 }, /* core_bi_px_core_in_mx_gpio_35 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 565 }, /* bi_mx_ssc_12_aoss_mx_gpio_200 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 566 }, /* bi_mx_ssc_10_aoss_mx_gpio_198 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 567 }, /* bi_mx_lpass_12_aoss_mx_gpio_177 */

	/* Bit 56 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 568 }, /* core_bi_px_core_in_mx_gpio_39 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 569 }, /* core_bi_px_core_in_mx_gpio_148 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 570 }, /* core_bi_px_core_in_mx_gpio_150 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 571 }, /* core_bi_px_core_in_mx_gpio_43 */

	/* Bit 60 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 572 }, /* core_bi_px_core_in_mx_gpio_54 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 573 }, /* core_bi_px_core_in_mx_gpio_27 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 574 }, /* core_bi_px_core_in_mx_gpio_99 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 575 }, /* core_bi_px_core_in_mx_gpio_19 */

	/* Bit 64 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 576 }, /* core_bi_px_core_in_mx_gpio_8 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 577 }, /* core_bi_px_core_in_mx_gpio_88 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 578 }, /* aoss_irq_core_in_mx_gpio_32 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 579 }, /* bi_mx_ssc_28_aoss_mx_gpio_7 */

	/* Bit 68 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 580 }, /* bi_mx_ssc_7_aoss_mx_gpio_195 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 581 }, /* core_bi_px_core_in_mx_gpio_136 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 582 }, /* bi_mx_ssc_14_aoss_mx_gpio_202 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 583 }, /* core_bi_px_core_in_mx_gpio_12 */

	/* Bit 72 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 584 }, /* aoss_irq_core_in_mx_gpio_56 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 585 }, /* aoss_irq_core_in_mx_gpio_155 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 586 }, /* core_bi_px_core_in_mx_gpio_153 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 587 }, /* core_bi_px_core_in_mx_gpio_18 */

	/* Bit 76 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 588 }, /* bi_mx_ssc_16_aoss_mx_gpio_204 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 589 }, /* aoss_irq_core_in_mx_gpio_57 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 590 }, /* bi_mx_ssc_22_aoss_mx_gpio_4 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 591 }, /* core_bi_px_core_in_mx_gpio_46 */

	/* Bit 80 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 592 }, /* core_bi_px_core_in_mx_gpio_133 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 593 }, /* core_bi_px_core_in_mx_gpio_137 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 594 }, /* core_bi_px_core_in_mx_gpio_15 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 595 }, /* core_bi_px_core_in_mx_gpio_67 */

	/* Bit 84 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 596 }, /* core_bi_px_core_in_mx_gpio_23 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 597 }, /* core_bi_px_core_in_mx_gpio_63 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 598 }, /* core_bi_px_core_in_mx_gpio_162 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 599 }, /* core_bi_px_core_in_mx_gpio_86 */

	/* Bit 88 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 600 }, /* core_bi_px_core_in_mx_gpio_87 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 601 }, /* core_bi_px_core_in_mx_gpio_80 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 602 }, /* core_bi_px_core_in_mx_gpio_81 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 603 }, /* bi_mx_ssc_17_aoss_mx_gpio_205 */

	/* Bit 92 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 604 }, /* core_bi_px_core_in_mx_gpio_90 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 605 }, /* core_bi_px_core_in_mx_gpio_51 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 641 }, /* bi_mx_ssc_20_aoss_mx_gpio_0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 642 }, /* bi_mx_lpass_22_aoss_mx_gpio_187 */

	/* Bit 96 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 643 }, /* core_bi_px_core_in_mx_gpio_75 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 644 }, /* core_bi_px_core_in_mx_gpio_76 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 645 }, /* core_bi_px_core_in_mx_gpio_77 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 646 }, /* core_bi_px_core_in_mx_gpio_92 */

	/* Bit 100 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 647 }, /* core_bi_px_core_in_mx_gpio_85 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 648 }, /* core_bi_px_core_in_mx_gpio_98 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 649 }, /* bi_mx_lpass_9_aoss_mx_gpio_174 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 650 }, /* bi_mx_ssc_11_aoss_mx_gpio_199 */

	/* Bit 104 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 651 }, /* core_bi_px_core_in_mx_gpio_55 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 652 }, /* bi_mx_ssc_26_aoss_mx_gpio_3 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 653 }, /* core_bi_px_core_in_mx_gpio_84 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 654 }, /* core_bi_px_core_in_mx_gpio_64 */

	/* Bit 108 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 655 }, /* core_bi_px_core_in_mx_gpio_65 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 656 }, /* core_bi_px_core_in_mx_gpio_66 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 657 }, /* core_bi_px_core_in_mx_gpio_68 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 658 }, /* core_bi_px_core_in_mx_gpio_69 */

	/* Bit 112 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 659 }, /* core_bi_px_core_in_mx_gpio_29 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 660 }, /* core_bi_px_core_in_mx_gpio_31 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 661 }, /* core_bi_px_core_in_mx_gpio_20 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 662 }, /* core_bi_px_core_in_mx_sdc2_data_3 */

	/* Bit 116 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 663 }, /* core_bi_px_core_in_mx_sdc2_data_1 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 664 }, /* core_bi_px_core_in_mx_sdc2_cmd */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 665 }, /* core_bi_px_core_in_mx_gpio_95 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 666 }, /* core_bi_px_core_in_mx_gpio_96 */

	/* Bit 120 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 667 }, /* core_bi_px_core_in_mx_gpio_112 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 668 }, /* core_bi_px_core_in_mx_gpio_11 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 669 }, /* core_bi_px_core_in_mx_gpio_163 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 670 }, /* aoss_irq_core_in_mx_gpio_36 */

	/* Bit 124 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 671 }, /* aoss_irq_core_in_mx_gpio_47 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 95 }, /* aoss_irq_core_in_mx_gpio_48 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 748 }, /* aoss_irq_core_in_mx_gpio_154 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 749 }, /* aoss_irq_core_in_mx_gpio_152 */

	/* Bit 128 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 750 }, /* aoss_irq_core_in_mx_gpio_156 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 751 }, /* aoss_irq_core_in_mx_gpio_159 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 752 }, /* bi_mx_ssc_0_aoss_mx_gpio_188 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 753 }, /* bi_mx_ssc_2_aoss_mx_gpio_190 */

	/* Bit 132 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 754 }, /* bi_mx_ssc_3_aoss_mx_gpio_191 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 755 }, /* bi_mx_ssc_4_aoss_mx_gpio_192 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 756 }, /* bi_mx_ssc_5_aoss_mx_gpio_193 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 757 }, /* bi_mx_ssc_8_aoss_mx_gpio_196 */

	/* Bit 136 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 758 }, /* bi_mx_ssc_9_aoss_mx_gpio_197 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 759 }, /* bi_mx_ssc_13_aoss_mx_gpio_201 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 283 }, /* bi_mx_ssc_15_aoss_mx_gpio_203 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 284 }, /* bi_mx_lpass_1_aoss_mx_gpio_166 */

	/* Bit 140 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 285 }, /* bi_mx_lpass_4_aoss_mx_gpio_169 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 286 }, /* bi_mx_lpass_6_aoss_mx_gpio_171 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 287 }, /* bi_mx_lpass_7_aoss_mx_gpio_172 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 276 }, /* bi_mx_lpass_11_aoss_mx_gpio_176 */

	/* Bit 144 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 277 }, /* bi_mx_lpass_16_aoss_mx_gpio_181 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 278 }, /* bi_mx_lpass_17_aoss_mx_gpio_182 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 279 }, /* bi_mx_lpass_20_aoss_mx_gpio_185 */
};

const uint32_t g_pdc_interrupt_table_size = ARRAY_SIZE(g_pdc_interrupt_mapping);
