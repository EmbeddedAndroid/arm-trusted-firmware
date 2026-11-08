/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/pdc/pdc_internal.h>
#include <lib/utils_def.h>

/*
 * PDC interrupt mapping for SM7750 APSS: the HLOS DRV owns every input.
 * The inputs without a GIC line keep interrupt 0.
 */
struct pdc_interrupt_mapping g_pdc_interrupt_mapping[] = {
	/* Bit 0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 512 }, /* spmi_protocol_irq[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 513 }, /* ee0_apps_hlos_spmi_periph_irq_0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 514 }, /* ee1_apps_trustzone_spmi_periph_irq_0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 515 }, /* ee0_apps_hlos_spmi_periph_irq_1 */

	/* Bit 4 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 516 }, /* ee1_apps_trustzone_spmi_periph_irq_1 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 517 }, /* aop_wdog_expired_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 518 }, /* wdog_exp_irq_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 519 }, /* lpass_sdc_gpo_bit0 */

	/* Bit 8 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 751 }, /* xpu_irq_summary_apps */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 750 }, /* vmidmt_irq_summary_apps */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 262 }, /* o_i3c_ibi_irq[26] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 756 }, /* eud_int_mx[1] */

	/* Bit 12 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 748 }, /* lpi_lb_interrupt[0] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 759 }, /* lpi_lb_interrupt[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 752 }, /* eud_p0_dpse_int_mx */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 758 }, /* eud_p0_dmse_int_mx */

	/* Bit 16 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 753 }, /* codec_tx_swr_irq[1] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 294 }, /* qmp_usb3_lfps_rxterm_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 102 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 755 },

	/* Bit 20 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 266 }, /* tsens_0C_summ_int */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 757 }, /* pdc_apps_epcb_timeout_summary_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 263 }, /* spmi_protocol_irq[0] */

	/* Bit 24 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 536 }, /* o_i3c_ibi_irq[6] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 537 }, /* o_i3c_ibi_irq[10] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 538 }, /* tsense0_upper_lower_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 539 }, /* tsense1_upper_lower_intr */

	/* Bit 28 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 540 }, /* tsense2_upper_lower_intr */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
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
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },

	/* Bit 40 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 552 }, /* aoss_wakeup_gpio_139 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 553 }, /* o_i3c_ibi_irq[2] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 554 }, /* o_i3c_ibi_irq[14] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 555 }, /* o_i3c_ibi_irq[18] */

	/* Bit 44 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 556 }, /* o_i3c_ibi_irq[22] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 557 }, /* LPI AON NoC sideband manager fault */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },

	/* Bit 48 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 563 }, /* o_i3c_ibi_irq[30] */

	/* Bit 52 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 564 }, /* o_i3c_ibi_irq[34] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 565 }, /* o_i3c_ibi_irq[38] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 566 }, /* prb_dma_crm_timeout_irq */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },

	/* Bit 56 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 570 }, /* prb_dma_irq_aoss */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 571 }, /* aad_irq_aoss */

	/* Bit 60 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 0 },
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 573 }, /* i3c_ibi_irq_s0[3] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 574 }, /* i3c_ibi_irq_s1[3] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 575 }, /* i3c_ibi_irq_s2[3] */

	/* Bit 64 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 576 }, /* i3c_ibi_irq_s3[3] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 577 }, /* i3c_ibi_irq_s4[3] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 124 }, /* i3c_ibi_irq_s5[3] */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 579 }, /* aoss_wakeup_gpio_91 */

	/* Bit 68 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 580 }, /* aoss_wakeup_gpio_6 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 581 }, /* aoss_wakeup_gpio_81 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 582 }, /* aoss_wakeup_gpio_17 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 583 }, /* aoss_wakeup_gpio_58 */

	/* Bit 72 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 584 }, /* aoss_wakeup_gpio_147 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 585 }, /* aoss_wakeup_gpio_171 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 586 }, /* aoss_wakeup_gpio_27 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 587 }, /* aoss_wakeup_gpio_49 */

	/* Bit 76 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 588 }, /* aoss_wakeup_gpio_79 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 589 }, /* aoss_wakeup_gpio_117 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 590 }, /* aoss_wakeup_gpio_129 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 591 }, /* aoss_wakeup_gpio_19 */

	/* Bit 80 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 272 }, /* aoss_wakeup_gpio_23 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 267 }, /* aoss_wakeup_gpio_120 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 342 }, /* aoss_wakeup_gpio_0 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 343 }, /* aoss_wakeup_gpio_100 */

	/* Bit 84 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 280 }, /* aoss_wakeup_gpio_109 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 273 }, /* aoss_wakeup_gpio_11 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 270 }, /* aoss_wakeup_gpio_112 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 271 }, /* aoss_wakeup_gpio_3 */

	/* Bit 88 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 286 }, /* aoss_wakeup_gpio_16 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 541 }, /* aoss_wakeup_gpio_39 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 595 }, /* aoss_wakeup_gpio_4 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 291 }, /* aoss_wakeup_gpio_26 */

	/* Bit 92 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 292 }, /* aoss_wakeup_gpio_113 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 233 }, /* aoss_wakeup_gpio_114 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 278 }, /* aoss_wakeup_gpio_116 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 125 }, /* aoss_wakeup_gpio_119 */

	/* Bit 96 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 643 }, /* aoss_wakeup_gpio_121 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 644 }, /* aoss_wakeup_gpio_122 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 645 }, /* aoss_wakeup_gpio_35 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 646 }, /* aoss_wakeup_gpio_63 */

	/* Bit 100 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 647 }, /* aoss_wakeup_gpio_34 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 648 }, /* aoss_wakeup_gpio_44 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 649 }, /* aoss_wakeup_gpio_123 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 650 }, /* aoss_wakeup_gpio_125 */

	/* Bit 104 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 651 }, /* aoss_wakeup_gpio_127 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 652 }, /* aoss_wakeup_gpio_128 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 653 }, /* aoss_wakeup_gpio_13 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 654 }, /* aoss_wakeup_gpio_12 */

	/* Bit 108 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 655 }, /* aoss_wakeup_gpio_118 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 656 }, /* aoss_wakeup_gpio_145 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 657 }, /* aoss_wakeup_gpio_115 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 658 }, /* aoss_wakeup_gpio_149 */

	/* Bit 112 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 659 }, /* aoss_wakeup_gpio_130 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 660 }, /* aoss_wakeup_gpio_131 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 661 }, /* aoss_wakeup_gpio_133 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 662 }, /* aoss_wakeup_gpio_135 */

	/* Bit 116 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 663 }, /* aoss_wakeup_gpio_139 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 664 }, /* aoss_wakeup_gpio_32 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 665 }, /* aoss_wakeup_gpio_142 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 666 }, /* aoss_wakeup_gpio_157 */

	/* Bit 120 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 667 }, /* aoss_wakeup_gpio_159 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 668 }, /* aoss_wakeup_gpio_161 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 669 }, /* aoss_wakeup_gpio_154 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 670 }, /* aoss_wakeup_gpio_164 */

	/* Bit 124 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 671 }, /* aoss_wakeup_gpio_165 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 95 }, /* aoss_wakeup_gpio_167 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 398 }, /* aoss_wakeup_gpio_170 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 399 }, /* aoss_wakeup_gpio_172 */

	/* Bit 128 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 406 }, /* aoss_wakeup_gpio_173 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 409 }, /* aoss_wakeup_gpio_174 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 460 }, /* aoss_wakeup_gpio_175 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 466 }, /* aoss_wakeup_gpio_176 */

	/* Bit 132 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 467 }, /* aoss_wakeup_gpio_177 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 469 }, /* aoss_wakeup_gpio_179 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 484 }, /* aoss_wakeup_gpio_18 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 485 }, /* aoss_wakeup_gpio_182 */

	/* Bit 136 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 490 }, /* aoss_wakeup_gpio_184 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 491 }, /* aoss_wakeup_gpio_28 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 496 }, /* aoss_wakeup_gpio_29 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 497 }, /* aoss_wakeup_gpio_30 */

	/* Bit 140 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 498 }, /* aoss_wakeup_gpio_31 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 499 }, /* aoss_wakeup_gpio_36 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 500 }, /* aoss_wakeup_gpio_40 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 501 }, /* aoss_wakeup_gpio_42 */

	/* Bit 144 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 502 }, /* aoss_wakeup_gpio_45 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 503 }, /* aoss_wakeup_gpio_46 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 504 }, /* aoss_wakeup_gpio_47 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 505 }, /* aoss_wakeup_gpio_51 */

	/* Bit 148 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 506 }, /* aoss_wakeup_gpio_52 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 703 }, /* aoss_wakeup_gpio_53 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 720 }, /* aoss_wakeup_gpio_54 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 746 }, /* aoss_wakeup_gpio_55 */

	/* Bit 152 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 747 }, /* aoss_wakeup_gpio_56 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 754 }, /* aoss_wakeup_gpio_7 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 287 }, /* aoss_wakeup_gpio_108 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 301 }, /* aoss_wakeup_gpio_59 */

	/* Bit 156 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 302 }, /* aoss_wakeup_gpio_78 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 308 }, /* aoss_wakeup_gpio_80 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 319 }, /* aoss_wakeup_gpio_87 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 338 }, /* aoss_wakeup_gpio_92 */

	/* Bit 160 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 339 }, /* aoss_wakeup_gpio_95 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 340 }, /* aoss_wakeup_gpio_98 */
	{ { TRIGGER_RISING_EDGE, PDC_DRV2 }, 341 }, /* aoss_wakeup_gpio_99 */
};

const uint32_t g_pdc_interrupt_table_size = ARRAY_SIZE(g_pdc_interrupt_mapping);
