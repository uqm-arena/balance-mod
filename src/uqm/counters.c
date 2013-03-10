// D Yu Okunev <xai@mephi.ru>
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "init.h"
#include "counters.h"
#include "libs/log.h"

#define METRIC_INITIAL (-1)
#define METRIC_ZERO (1E-10)
#define METRIC_LOSS (1E+6)
#define METRIC_MIDDLE(ship_cost) (WINNING_PROBABILITY_TABLE[NO_ID][NO_ID]*ship_cost)

#define MAX_SKIPS	3

// [Warping in ID0][Staying ID1], ID0 winning probability
static float WINNING_PROBABILITY_TABLE[NUM_SPECIES_ID][NUM_SPECIES_ID] = {
	{ // NO_ID
		0.50,	// NO_ID
		0,	// ARILOU_ID
		0,	// CHMMR_ID
		0,	// EARTHLING_ID
		0,	// ORZ_ID
		0,	// PKUNK_ID
		0,	// SHOFIXTI_ID
		0,	// SPATHI_ID
		0,	// SUPOX_ID
		0,	// THRADDASH_ID
		0,	// UTWIG_ID
		0,	// VUX_ID
		0,	// YEHAT_ID
		0,	// MELNORME_ID
		0,	// DRUUGE_ID
		0,	// ILWRATH_ID
		0,	// MYCON_ID
		0,	// SLYLANDRO_ID
		0,	// UMGAH_ID
		0,	// UR_QUAN_ID
		0,	// ZOQFOTPIK_ID
		0,	// SYREEN_ID
		0,	// KOHR_AH_ID
		0,	// ANDROSYNTH_ID
		0,	// CHENJESU_ID
		0,	// MMRNMHRM_ID
		0,	// SIS_SHIP_ID
		0,	// SA_MATRA_ID
		0,	// UR_QUAN_PROBE_ID
 	},
	{ // ARILOU_ID
		1,	// NO_ID
		0.50,	// ARILOU_ID
		0.05,	// CHMMR_ID
		
		/* Arilou is fairly likely to beat Earthling in PvP, but
		 * the AI currently approaches the match rather stupidly.
		  */
		0.10,	// EARTHLING_ID
		0.70,	// ORZ_ID
		0.20,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.60,	// SPATHI_ID
		0.70,	// SUPOX_ID
		0.70,	// THRADDASH_ID
		0.35,	// UTWIG_ID
		0.40,	// VUX_ID
		0.50,	// YEHAT_ID
		0.65,	// MELNORME_ID
		0.70,	// DRUUGE_ID
		0.40,	// ILWRATH_ID
		0.95,	// MYCON_ID
		0.30,	// SLYLANDRO_ID
		0.90,	// UMGAH_ID
		0.20,	// UR_QUAN_ID
		0.70,	// ZOQFOTPIK_ID
		0.60,	// SYREEN_ID
		0.20,	// KOHR_AH_ID
		0.40,	// ANDROSYNTH_ID
		0.30,	// CHENJESU_ID
		0.30,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.20,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
 	},
 	{ // CHMMR_ID
		1,	// NO_ID
		0.90,	// ARILOU_ID
		0.50,	// CHMMR_ID
		0.99,	// EARTHLING_ID
		0.50,	// ORZ_ID
		0.75,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.65,	// SPATHI_ID
		0.95,	// SUPOX_ID
		0.80,	// THRADDASH_ID
		0.30,	// UTWIG_ID
		0.90,	// VUX_ID
		0.70,	// YEHAT_ID
		0.85,	// MELNORME_ID
		0.40,	// DRUUGE_ID
		0.95,	// ILWRATH_ID
		0.90,	// MYCON_ID
		0.80,	// SLYLANDRO_ID
		0.95,	// UMGAH_ID
		0.60,	// UR_QUAN_ID
		0.99,	// ZOQFOTPIK_ID
		0.90,	// SYREEN_ID
		0.60,	// KOHR_AH_ID
		0.70,	// ANDROSYNTH_ID
		0.70,	// CHENJESU_ID
		0.70,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.20,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // EARTHLING_ID
		1,	// NO_ID
		0.10,	// ARILOU_ID
		0.05,	// CHMMR_ID
		0.50,	// EARTHLING_ID
		0.20,	// ORZ_ID
		0.05,	// PKUNK_ID
		0.10,	// SHOFIXTI_ID
		0.10,	// SPATHI_ID
		0.10,	// SUPOX_ID
		0.10,	// THRADDASH_ID
		0.05,	// UTWIG_ID
		0.20,	// VUX_ID
		0.10,	// YEHAT_ID
		0.30,	// MELNORME_ID
		0.50,	// DRUUGE_ID
		0.20,	// ILWRATH_ID
		0.60,	// MYCON_ID
		0.05,	// SLYLANDRO_ID
		0.15,	// UMGAH_ID
		0.05,	// UR_QUAN_ID
		0.15,	// ZOQFOTPIK_ID
		0.50,	// SYREEN_ID
		0.05,	// KOHR_AH_ID
		0.10,	// ANDROSYNTH_ID
		0.15,	// CHENJESU_ID
		0.10,	// MMRNMHRM_ID
		0.01,	// SIS_SHIP_ID
		0.01,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // ORZ_ID
		1,	// NO_ID
		0.20,	// ARILOU_ID
		0.55,	// CHMMR_ID
		0.80,	// EARTHLING_ID
		0.60,	// ORZ_ID
		0.10,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.40,	// SPATHI_ID
		0.60,	// SUPOX_ID
		0.30,	// THRADDASH_ID
		0.80,	// UTWIG_ID
		0.60,	// VUX_ID
		0.75,	// YEHAT_ID
		0.80,	// MELNORME_ID
		0.80,	// DRUUGE_ID
		0.70,	// ILWRATH_ID
		0.85,	// MYCON_ID
		0.30,	// SLYLANDRO_ID
		0.40,	// UMGAH_ID
		0.35,	// UR_QUAN_ID
		0.30,	// ZOQFOTPIK_ID
		0.70,	// SYREEN_ID
		0.50,	// KOHR_AH_ID
		0.05,	// ANDROSYNTH_ID
		0.65,	// CHENJESU_ID
		0.25,	// MMRNMHRM_ID
		0.30,	// SIS_SHIP_ID
		0.20,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // PKUNK_ID
		1,	// NO_ID
		0.70,	// ARILOU_ID
		0.20,	// CHMMR_ID
		0.80,	// EARTHLING_ID
		0.70,	// ORZ_ID
		0.50,	// PKUNK_ID
		0.80,	// SHOFIXTI_ID
		0.40,	// SPATHI_ID
		0.40,	// SUPOX_ID
		0.70,	// THRADDASH_ID
		0.10,	// UTWIG_ID
		0.50,	// VUX_ID
		0.20,	// YEHAT_ID
		0.75,	// MELNORME_ID
		0.85,	// DRUUGE_ID
		0.40,	// ILWRATH_ID
		0.90,	// MYCON_ID
		0.30,	// SLYLANDRO_ID
		0.70,	// UMGAH_ID
		0.20,	// UR_QUAN_ID
		0.70,	// ZOQFOTPIK_ID
		0.80,	// SYREEN_ID
		0.50,	// KOHR_AH_ID
		0.70,	// ANDROSYNTH_ID
		0.35,	// CHENJESU_ID
		0.50,	// MMRNMHRM_ID
		0.30,	// SIS_SHIP_ID
		1.00,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // SHOFIXTI_ID
		1,	// NO_ID
		0.50,	// ARILOU_ID
		0.50,	// CHMMR_ID
		0.40,	// EARTHLING_ID
		0.50,	// ORZ_ID
		0.25,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.10,	// SPATHI_ID
		0.30,	// SUPOX_ID
		0.25,	// THRADDASH_ID
		0.25,	// UTWIG_ID
		0.45,	// VUX_ID
		0.25,	// YEHAT_ID
		0.40,	// MELNORME_ID
		0.50,	// DRUUGE_ID
		0.30,	// ILWRATH_ID
		0.65,	// MYCON_ID
		0.20,	// SLYLANDRO_ID
		0.35,	// UMGAH_ID
		0.20,	// UR_QUAN_ID
		0.35,	// ZOQFOTPIK_ID
		0.45,	// SYREEN_ID
		0.15,	// KOHR_AH_ID
		0.25,	// ANDROSYNTH_ID
		0.25,	// CHENJESU_ID
		0.35,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // SPATHI_ID
		1,	// NO_ID
		0.40,	// ARILOU_ID
		0.30,	// CHMMR_ID
		0.60,	// EARTHLING_ID
		0.25,	// ORZ_ID
		0.25,	// PKUNK_ID
		0.90,	// SHOFIXTI_ID
		0.55,	// SPATHI_ID
		0.35,	// SUPOX_ID
		0.30,	// THRADDASH_ID
		0.20,	// UTWIG_ID
		0.85,	// VUX_ID
		0.80,	// YEHAT_ID
		0.50,	// MELNORME_ID
		0.35,	// DRUUGE_ID
		0.50,	// ILWRATH_ID
		0.90,	// MYCON_ID
		0.10,	// SLYLANDRO_ID
		0.70,	// UMGAH_ID
		0.10,	// UR_QUAN_ID
		0.30,	// ZOQFOTPIK_ID
		0.15,	// SYREEN_ID
		0.30,	// KOHR_AH_ID
		0.10,	// ANDROSYNTH_ID
		0.30,	// CHENJESU_ID
		0.10,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.15,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // SUPOX_ID
		1,	// NO_ID
		0.20,	// ARILOU_ID
		0.05,	// CHMMR_ID
		0.75,	// EARTHLING_ID
		0.65,	// ORZ_ID
		0.20,	// PKUNK_ID
		0.60,	// SHOFIXTI_ID
		0.60,	// SPATHI_ID
		0.55,	// SUPOX_ID
		0.30,	// THRADDASH_ID
		0.30,	// UTWIG_ID
		0.75,	// VUX_ID
		0.70,	// YEHAT_ID
		0.70,	// MELNORME_ID
		0.70,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.90,	// MYCON_ID
		0.30,	// SLYLANDRO_ID
		0.50,	// UMGAH_ID
		0.20,	// UR_QUAN_ID
		0.70,	// ZOQFOTPIK_ID
		0.60,	// SYREEN_ID
		0.35,	// KOHR_AH_ID
		0.40,	// ANDROSYNTH_ID
		0.40,	// CHENJESU_ID
		0.20,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.20,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // THRADDASH_ID
		1,	// NO_ID
		0.30,	// ARILOU_ID
		0.10,	// CHMMR_ID
		0.75,	// EARTHLING_ID
		0.70,	// ORZ_ID
		0.20,	// PKUNK_ID
		0.70,	// SHOFIXTI_ID
		0.30,	// SPATHI_ID
		0.35,	// SUPOX_ID
		0.55,	// THRADDASH_ID
		0.10,	// UTWIG_ID
		0.50,	// VUX_ID
		0.30,	// YEHAT_ID
		0.50,	// MELNORME_ID
		0.80,	// DRUUGE_ID
		0.45,	// ILWRATH_ID
		0.70,	// MYCON_ID
		0.10,	// SLYLANDRO_ID
		0.75,	// UMGAH_ID
		0.15,	// UR_QUAN_ID
		0.30,	// ZOQFOTPIK_ID
		0.50,	// SYREEN_ID
		0.10,	// KOHR_AH_ID
		0.50,	// ANDROSYNTH_ID
		0.45,	// CHENJESU_ID
		0.40,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.25,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // UTWIG_ID
		1,	// NO_ID
		0.75,	// ARILOU_ID
		0.80,	// CHMMR_ID
		0.80,	// EARTHLING_ID
		0.15,	// ORZ_ID
		0.70,	// PKUNK_ID
		0.85,	// SHOFIXTI_ID
		0.50,	// SPATHI_ID
		0.50,	// SUPOX_ID
		0.50,	// THRADDASH_ID
		0.50,	// UTWIG_ID
		0.15,	// VUX_ID
		0.70,	// YEHAT_ID
		0.30,	// MELNORME_ID
		0.55,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.95,	// MYCON_ID
		0.60,	// SLYLANDRO_ID
		0.55,	// UMGAH_ID
		0.15,	// UR_QUAN_ID
		0.85,	// ZOQFOTPIK_ID
		0.85,	// SYREEN_ID
		0.65,	// KOHR_AH_ID
		0.65,	// ANDROSYNTH_ID
		0.15,	// CHENJESU_ID
		0.30,	// MMRNMHRM_ID
		0.30,	// SIS_SHIP_ID
		0.70,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // VUX_ID
		1,	// NO_ID
		0.10,	// ARILOU_ID
		0.15,	// CHMMR_ID
		0.10,	// EARTHLING_ID
		0.50,	// ORZ_ID
		0.15,	// PKUNK_ID
		0.45,	// SHOFIXTI_ID
		0.25,	// SPATHI_ID
		0.50,	// SUPOX_ID
		0.50,	// THRADDASH_ID
		0.75,	// UTWIG_ID
		0.80,	// VUX_ID
		0.75,	// YEHAT_ID
		0.75,	// MELNORME_ID
		0.50,	// DRUUGE_ID
		0.50,	// ILWRATH_ID
		0.50,	// MYCON_ID
		0.10,	// SLYLANDRO_ID
		0.25,	// UMGAH_ID
		0.20,	// UR_QUAN_ID
		0.30,	// ZOQFOTPIK_ID
		0.50,	// SYREEN_ID
		0.15,	// KOHR_AH_ID
		0.15,	// ANDROSYNTH_ID
		0.35,	// CHENJESU_ID
		0.30,	// MMRNMHRM_ID
		0.35,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // YEHAT_ID
		1,	// NO_ID
		0.70,	// ARILOU_ID
		0.10,	// CHMMR_ID
		0.95,	// EARTHLING_ID
		0.15,	// ORZ_ID
		0.70,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.20,	// SPATHI_ID
		0.45,	// SUPOX_ID
		0.45,	// THRADDASH_ID
		0.25,	// UTWIG_ID
		0.05,	// VUX_ID
		0.55,	// YEHAT_ID
		0.35,	// MELNORME_ID
		0.90,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.95,	// MYCON_ID
		0.65,	// SLYLANDRO_ID
		0.55,	// UMGAH_ID
		0.05,	// UR_QUAN_ID
		0.75,	// ZOQFOTPIK_ID
		0.50,	// SYREEN_ID
		0.20,	// KOHR_AH_ID
		0.65,	// ANDROSYNTH_ID
		0.60,	// CHENJESU_ID
		0.10,	// MMRNMHRM_ID
		0.15,	// SIS_SHIP_ID
		0.35,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // MELNORME_ID
		1,	// NO_ID
		0.20,	// ARILOU_ID
		0.40,	// CHMMR_ID
		0.75,	// EARTHLING_ID
		0.55,	// ORZ_ID
		0.40,	// PKUNK_ID
		0.70,	// SHOFIXTI_ID
		0.50,	// SPATHI_ID
		0.50,	// SUPOX_ID
		0.60,	// THRADDASH_ID
		0.75,	// UTWIG_ID
		0.65,	// VUX_ID
		0.65,	// YEHAT_ID
		0.50,	// MELNORME_ID
		0.45,	// DRUUGE_ID
		0.80,	// ILWRATH_ID
		0.75,	// MYCON_ID
		0.35,	// SLYLANDRO_ID
		0.75,	// UMGAH_ID
		0.40,	// UR_QUAN_ID
		0.70,	// ZOQFOTPIK_ID
		0.65,	// SYREEN_ID
		0.40,	// KOHR_AH_ID
		0.30,	// ANDROSYNTH_ID
		0.50,	// CHENJESU_ID
		0.65,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.10,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // DRUUGE_ID
		1,	// NO_ID
		0.30,	// ARILOU_ID
		0.60,	// CHMMR_ID
		0.40,	// EARTHLING_ID
		0.40,	// ORZ_ID
		0.30,	// PKUNK_ID
		0.65,	// SHOFIXTI_ID
		0.60,	// SPATHI_ID
		0.50,	// SUPOX_ID
		0.55,	// THRADDASH_ID
		0.60,	// UTWIG_ID
		0.60,	// VUX_ID
		0.50,	// YEHAT_ID
		0.50,	// MELNORME_ID
		0.50,	// DRUUGE_ID
		0.10,	// ILWRATH_ID
		0.20,	// MYCON_ID
		0.35,	// SLYLANDRO_ID
		0.70,	// UMGAH_ID
		0.15,	// UR_QUAN_ID
		0.60,	// ZOQFOTPIK_ID
		0.65,	// SYREEN_ID
		0.15,	// KOHR_AH_ID
		0.35,	// ANDROSYNTH_ID
		0.30,	// CHENJESU_ID
		0.45,	// MMRNMHRM_ID
		0.25,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // ILWRATH_ID
		1,	// NO_ID
		0.60,	// ARILOU_ID
		0.10,	// CHMMR_ID
		0.75,	// EARTHLING_ID
		0.30,	// ORZ_ID
		0.40,	// PKUNK_ID
		0.60,	// SHOFIXTI_ID
		0.30,	// SPATHI_ID
		0.25,	// SUPOX_ID
		0.50,	// THRADDASH_ID
		0.20,	// UTWIG_ID
		0.40,	// VUX_ID
		0.20,	// YEHAT_ID
		0.30,	// MELNORME_ID
		0.30,	// DRUUGE_ID
		0.50,	// ILWRATH_ID
		0.65,	// MYCON_ID
		0.25,	// SLYLANDRO_ID
		0.40,	// UMGAH_ID
		0.15,	// UR_QUAN_ID
		0.45,	// ZOQFOTPIK_ID
		0.20,	// SYREEN_ID
		0.15,	// KOHR_AH_ID
		0.40,	// ANDROSYNTH_ID
		0.35,	// CHENJESU_ID
		0.30,	// MMRNMHRM_ID
		0.10,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // MYCON_ID
		1,	// NO_ID
		0.10,	// ARILOU_ID
		0.10,	// CHMMR_ID
		0.45,	// EARTHLING_ID
		0.15,	// ORZ_ID
		0.10,	// PKUNK_ID
		0.90,	// SHOFIXTI_ID
		0.25,	// SPATHI_ID
		0.25,	// SUPOX_ID
		0.30,	// THRADDASH_ID
		0.15,	// UTWIG_ID
		0.50,	// VUX_ID
		0.25,	// YEHAT_ID
		0.40,	// MELNORME_ID
		0.90,	// DRUUGE_ID
		0.40,	// ILWRATH_ID
		0.50,	// MYCON_ID
		0.10,	// SLYLANDRO_ID
		0.30,	// UMGAH_ID
		0.25,	// UR_QUAN_ID
		0.25,	// ZOQFOTPIK_ID
		0.35,	// SYREEN_ID
		0.45,	// KOHR_AH_ID
		0.25,	// ANDROSYNTH_ID
		0.40,	// CHENJESU_ID
		0.35,	// MMRNMHRM_ID
		0.10,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // SLYLANDRO_ID
		1,	// NO_ID
		0.60,	// ARILOU_ID
		0.05,	// CHMMR_ID
		0.75,	// EARTHLING_ID
		0.75,	// ORZ_ID
		0.60,	// PKUNK_ID
		0.80,	// SHOFIXTI_ID
		0.40,	// SPATHI_ID
		0.65,	// SUPOX_ID
		0.75,	// THRADDASH_ID
		0.35,	// UTWIG_ID
		0.85,	// VUX_ID
		0.65,	// YEHAT_ID
		0.80,	// MELNORME_ID
		0.80,	// DRUUGE_ID
		0.80,	// ILWRATH_ID
		0.90,	// MYCON_ID
		0.50,	// SLYLANDRO_ID
		0.70,	// UMGAH_ID
		0.15,	// UR_QUAN_ID
		0.70,	// ZOQFOTPIK_ID
		0.65,	// SYREEN_ID
		0.20,	// KOHR_AH_ID
		0.60,	// ANDROSYNTH_ID
		0.55,	// CHENJESU_ID
		0.50,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // UMGAH_ID
		1,	// NO_ID
		0.10,	// ARILOU_ID
		0.05,	// CHMMR_ID
		0.70,	// EARTHLING_ID
		0.40,	// ORZ_ID
		0.20,	// PKUNK_ID
		0.55,	// SHOFIXTI_ID
		0.30,	// SPATHI_ID
		0.40,	// SUPOX_ID
		0.40,	// THRADDASH_ID
		0.15,	// UTWIG_ID
		0.30,	// VUX_ID
		0.25,	// YEHAT_ID
		0.35,	// MELNORME_ID
		0.20,	// DRUUGE_ID
		0.45,	// ILWRATH_ID
		0.45,	// MYCON_ID
		0.15,	// SLYLANDRO_ID
		0.50,	// UMGAH_ID
		0.15,	// UR_QUAN_ID
		0.50,	// ZOQFOTPIK_ID
		0.40,	// SYREEN_ID
		0.15,	// KOHR_AH_ID
		0.20,	// ANDROSYNTH_ID
		0.30,	// CHENJESU_ID
		0.30,	// MMRNMHRM_ID
		0.05,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // UR_QUAN_ID
		1,	// NO_ID
		0.85,	// ARILOU_ID
		0.40,	// CHMMR_ID
		0.95,	// EARTHLING_ID
		0.35,	// ORZ_ID
		0.70,	// PKUNK_ID
		0.90,	// SHOFIXTI_ID
		0.80,	// SPATHI_ID
		0.85,	// SUPOX_ID
		0.90,	// THRADDASH_ID
		0.75,	// UTWIG_ID
		0.85,	// VUX_ID
		0.85,	// YEHAT_ID
		0.65,	// MELNORME_ID
		0.85,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.85,	// MYCON_ID
		0.90,	// SLYLANDRO_ID
		0.85,	// UMGAH_ID
		0.50,	// UR_QUAN_ID
		0.90,	// ZOQFOTPIK_ID
		0.85,	// SYREEN_ID
		0.50,	// KOHR_AH_ID
		0.75,	// ANDROSYNTH_ID
		0.40,	// CHENJESU_ID
		0.65,	// MMRNMHRM_ID
		0.25,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // ZOQFOTPIK_ID
		1,	// NO_ID
		0.40,	// ARILOU_ID
		0.10,	// CHMMR_ID
		0.70,	// EARTHLING_ID
		0.30,	// ORZ_ID
		0.25,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.30,	// SPATHI_ID
		0.30,	// SUPOX_ID
		0.45,	// THRADDASH_ID
		0.20,	// UTWIG_ID
		0.40,	// VUX_ID
		0.15,	// YEHAT_ID
		0.35,	// MELNORME_ID
		0.35,	// DRUUGE_ID
		0.55,	// ILWRATH_ID
		0.80,	// MYCON_ID
		0.25,	// SLYLANDRO_ID
		0.50,	// UMGAH_ID
		0.13,	// UR_QUAN_ID
		0.50,	// ZOQFOTPIK_ID
		0.35,	// SYREEN_ID
		0.10,	// KOHR_AH_ID
		0.25,	// ANDROSYNTH_ID
		0.25,	// CHENJESU_ID
		0.25,	// MMRNMHRM_ID
		0.15,	// SIS_SHIP_ID
		0.10,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // SYREEN_ID
		1,	// NO_ID
		0.40,	// ARILOU_ID
		0.10,	// CHMMR_ID
		0.65,	// EARTHLING_ID
		0.70,	// ORZ_ID
		0.50,	// PKUNK_ID
		0.65,	// SHOFIXTI_ID
		0.50,	// SPATHI_ID
		0.45,	// SUPOX_ID
		0.60,	// THRADDASH_ID
		0.35,	// UTWIG_ID
		0.60,	// VUX_ID
		0.55,	// YEHAT_ID
		0.30,	// MELNORME_ID
		0.30,	// DRUUGE_ID
		0.70,	// ILWRATH_ID
		0.65,	// MYCON_ID
		0.25,	// SLYLANDRO_ID
		0.60,	// UMGAH_ID
		0.15,	// UR_QUAN_ID
		0.65,	// ZOQFOTPIK_ID
		0.50,	// SYREEN_ID
		0.15,	// KOHR_AH_ID
		0.35,	// ANDROSYNTH_ID
		0.35,	// CHENJESU_ID
		0.65,	// MMRNMHRM_ID
		0.10,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // KOHR_AH_ID
		1,	// NO_ID
		0.80,	// ARILOU_ID
		0.50,	// CHMMR_ID
		0.90,	// EARTHLING_ID
		0.80,	// ORZ_ID
		0.80,	// PKUNK_ID
		0.90,	// SHOFIXTI_ID
		0.85,	// SPATHI_ID
		0.85,	// SUPOX_ID
		0.90,	// THRADDASH_ID
		0.15,	// UTWIG_ID
		0.90,	// VUX_ID
		0.80,	// YEHAT_ID
		0.65,	// MELNORME_ID
		0.85,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.68,	// MYCON_ID
		0.85,	// SLYLANDRO_ID
		0.90,	// UMGAH_ID
		0.50,	// UR_QUAN_ID
		0.85,	// ZOQFOTPIK_ID
		0.90,	// SYREEN_ID
		0.45,	// KOHR_AH_ID
		0.75,	// ANDROSYNTH_ID
		0.65,	// CHENJESU_ID
		0.65,	// MMRNMHRM_ID
		0.30,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // ANDROSYNTH_ID
		1,	// NO_ID
		0.50,	// ARILOU_ID
		0.15,	// CHMMR_ID
		0.85,	// EARTHLING_ID
		0.95,	// ORZ_ID
		0.40,	// PKUNK_ID
		0.70,	// SHOFIXTI_ID
		0.70,	// SPATHI_ID
		0.65,	// SUPOX_ID
		0.70,	// THRADDASH_ID
		0.20,	// UTWIG_ID
		0.80,	// VUX_ID
		0.15,	// YEHAT_ID
		0.70,	// MELNORME_ID
		0.70,	// DRUUGE_ID
		0.65,	// ILWRATH_ID
		0.70,	// MYCON_ID
		0.30,	// SLYLANDRO_ID
		0.70,	// UMGAH_ID
		0.20,	// UR_QUAN_ID
		0.75,	// ZOQFOTPIK_ID
		0.60,	// SYREEN_ID
		0.15,	// KOHR_AH_ID
		0.45,	// ANDROSYNTH_ID
		0.50,	// CHENJESU_ID
		0.50,	// MMRNMHRM_ID
		0.15,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // CHENJESU_ID
		1,	// NO_ID
		0.70,	// ARILOU_ID
		0.35,	// CHMMR_ID
		0.80,	// EARTHLING_ID
		0.35,	// ORZ_ID
		0.50,	// PKUNK_ID
		0.90,	// SHOFIXTI_ID
		0.75,	// SPATHI_ID
		0.70,	// SUPOX_ID
		0.80,	// THRADDASH_ID
		0.75,	// UTWIG_ID
		0.35,	// VUX_ID
		0.50,	// YEHAT_ID
		0.50,	// MELNORME_ID
		0.80,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.70,	// MYCON_ID
		0.55,	// SLYLANDRO_ID
		0.80,	// UMGAH_ID
		0.40,	// UR_QUAN_ID
		0.80,	// ZOQFOTPIK_ID
		0.75,	// SYREEN_ID
		0.30,	// KOHR_AH_ID
		0.55,	// ANDROSYNTH_ID
		0.55,	// CHENJESU_ID
		0.45,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.05,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // MMRNMHRM_ID
		1,	// NO_ID
		0.65,	// ARILOU_ID
		0.10,	// CHMMR_ID
		0.75,	// EARTHLING_ID
		0.65,	// ORZ_ID
		0.60,	// PKUNK_ID
		0.90,	// SHOFIXTI_ID
		0.75,	// SPATHI_ID
		0.60,	// SUPOX_ID
		0.75,	// THRADDASH_ID
		0.80,	// UTWIG_ID
		0.90,	// VUX_ID
		0.75,	// YEHAT_ID
		0.50,	// MELNORME_ID
		0.70,	// DRUUGE_ID
		0.60,	// ILWRATH_ID
		0.85,	// MYCON_ID
		0.60,	// SLYLANDRO_ID
		0.90,	// UMGAH_ID
		0.30,	// UR_QUAN_ID
		0.75,	// ZOQFOTPIK_ID
		0.25,	// SYREEN_ID
		0.15,	// KOHR_AH_ID
		0.30,	// ANDROSYNTH_ID
		0.65,	// CHENJESU_ID
		0.50,	// MMRNMHRM_ID
		0.15,	// SIS_SHIP_ID
		0.10,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // SIS_SHIP_ID
		1,	// NO_ID
		0.80,	// ARILOU_ID
		0.50,	// CHMMR_ID
		0.90,	// EARTHLING_ID
		0.80,	// ORZ_ID
		0.65,	// PKUNK_ID
		0.90,	// SHOFIXTI_ID
		0.80,	// SPATHI_ID
		0.80,	// SUPOX_ID
		0.80,	// THRADDASH_ID
		0.40,	// UTWIG_ID
		0.40,	// VUX_ID
		0.60,	// YEHAT_ID
		0.60,	// MELNORME_ID
		0.70,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.80,	// MYCON_ID
		0.40,	// SLYLANDRO_ID
		0.70,	// UMGAH_ID
		0.30,	// UR_QUAN_ID
		0.80,	// ZOQFOTPIK_ID
		0.80,	// SYREEN_ID
		0.30,	// KOHR_AH_ID
		0.50,	// ANDROSYNTH_ID
		0.40,	// CHENJESU_ID
		0.60,	// MMRNMHRM_ID
		0.50,	// SIS_SHIP_ID
		0.20,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // SA_MATRA_ID
		1,	// NO_ID
		0.50,	// ARILOU_ID
		0.50,	// CHMMR_ID
		0.50,	// EARTHLING_ID
		0.50,	// ORZ_ID
		0.50,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.50,	// SPATHI_ID
		0.50,	// SUPOX_ID
		0.50,	// THRADDASH_ID
		0.50,	// UTWIG_ID
		0.50,	// VUX_ID
		0.50,	// YEHAT_ID
		0.50,	// MELNORME_ID
		0.50,	// DRUUGE_ID
		0.50,	// ILWRATH_ID
		0.50,	// MYCON_ID
		0.50,	// SLYLANDRO_ID
		0.50,	// UMGAH_ID
		0.50,	// UR_QUAN_ID
		0.50,	// ZOQFOTPIK_ID
		0.50,	// SYREEN_ID
		0.50,	// KOHR_AH_ID
		0.50,	// ANDROSYNTH_ID
		0.50,	// CHENJESU_ID
		0.50,	// MMRNMHRM_ID
		0.50,	// SIS_SHIP_ID
		0.50,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // UR_QUAN_PROBE_ID
		1,	// NO_ID
		0.50,	// ARILOU_ID
		0.50,	// CHMMR_ID
		0.50,	// EARTHLING_ID
		0.50,	// ORZ_ID
		0.50,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.50,	// SPATHI_ID
		0.50,	// SUPOX_ID
		0.50,	// THRADDASH_ID
		0.50,	// UTWIG_ID
		0.50,	// VUX_ID
		0.50,	// YEHAT_ID
		0.50,	// MELNORME_ID
		0.50,	// DRUUGE_ID
		0.50,	// ILWRATH_ID
		0.50,	// MYCON_ID
		0.50,	// SLYLANDRO_ID
		0.50,	// UMGAH_ID
		0.50,	// UR_QUAN_ID
		0.50,	// ZOQFOTPIK_ID
		0.50,	// SYREEN_ID
		0.50,	// KOHR_AH_ID
		0.50,	// ANDROSYNTH_ID
		0.50,	// CHENJESU_ID
		0.50,	// MMRNMHRM_ID
		0.50,	// SIS_SHIP_ID
		0.50,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
}; 

inline float 
_counter_getBest_calcLocalMetric(SPECIES_ID my_ID, SPECIES_ID enemy_ID, BYTE my_ship_cost)
{
	float metric;
	metric  = 1/(WINNING_PROBABILITY_TABLE[my_ID][enemy_ID]+METRIC_ZERO);
	metric *= metric;
	metric *= my_ship_cost;
	return metric;
}

float
_counter_getBest_getMetric_recursive(SIZE my_playerNr, HSTARSHIP my_hShip, HSTARSHIP hShip, /*SPECIES_ID my_ID, COUNT idx, */int my_skips, int enemy_skips, SIZE pick_playerNr, float *p_metric_enemy, /*STARSHIP *EnemyShipPtr,*/ COUNT enemy_idx_already, SPECIES_ID enemy_ID_already)
{
	int ships_left;
	static COUNT skip[NUM_PLAYERS][MAX_SHIPS_PER_SIDE];
	SIZE enemy_playerNr = !my_playerNr;
	QUEUE *enemy_ship_q = &race_q[enemy_playerNr]/*, *enemy_ship_q = &race_q[enemy_playerNr]*/;
	SPECIES_ID enemy_ID;
	HSTARSHIP enemy_hShip, enemy_hNextShip/*, enemy_hShip, enemy_hNextShip*/;
	float metric_best, metric, metric_continue, metric_enemy, metric_enemy_best, metric_my;

	QUEUE *my_ship_q 	= &race_q[my_playerNr];
	STARSHIP *my_StarShipPtr = LockStarShip (my_ship_q, my_hShip);
	//COUNT my_idx 		= my_StarShipPtr->index;
	SPECIES_ID my_ID  	= my_StarShipPtr->SpeciesID;
	BYTE ship_cost 		= my_StarShipPtr->ship_cost;
	UnlockStarShip (my_ship_q, my_hShip);

	QUEUE *ship_q = &race_q[my_playerNr == pick_playerNr ? my_playerNr : enemy_playerNr];
	STARSHIP *StarShipPtr = LockStarShip (ship_q, hShip);
	COUNT idx = StarShipPtr->index;
	UnlockStarShip (ship_q, hShip);

	if(my_playerNr == pick_playerNr)
		skip[my_playerNr	][my_skips++	] = idx;
	else
		skip[enemy_playerNr	][enemy_skips++	] = idx;

	
	if(my_skips >= MAX_SKIPS || enemy_skips >= MAX_SKIPS)
		return METRIC_MIDDLE(ship_cost);

	ships_left		= 0;
	metric_best		= METRIC_INITIAL;
	metric_enemy_best	= METRIC_INITIAL;

//	log_add (log_Debug, "recursive: %i %i\n", my_skips, enemy_skips);

	// TODO: consider with reserve variants if primary way was broken off [analog of: 1/R = 1/R1 + 1/R2]
	for (enemy_hShip = GetHeadLink (enemy_ship_q); enemy_hShip != 0; enemy_hShip = enemy_hNextShip)
	{
		COUNT enemy_idx;
		char skipthisship, lastship;
		STARSHIP *enemy_StarShipPtr;

		lastship=0;
		skipthisship=0;
		if(enemy_idx_already != MAX_SHIPS_PER_SIDE) {
			// TODO: find-out what is with lock-up mechanisms
			enemy_idx = enemy_idx_already;
			enemy_ID  = enemy_ID_already;
/*
			enemy_StarShipPtr = EnemyShipPtr;
			enemy_idx = enemy_StarShipPtr->index;
			enemy_ID  = enemy_StarShipPtr->SpeciesID;
*/
//			log_add (log_Debug, "enemy ID: %i\n", enemy_ID);
			lastship=1;
		} else {
			enemy_StarShipPtr = LockStarShip (enemy_ship_q, enemy_hShip);
			enemy_idx = enemy_StarShipPtr->index;
			enemy_ID  = enemy_StarShipPtr->SpeciesID;
			enemy_hNextShip = _GetSuccLink (enemy_StarShipPtr);
			UnlockStarShip (enemy_ship_q, enemy_hShip);

			if(enemy_ID == NO_ID) {
				skipthisship=1;
			} else {
				int i;
				i=0;
				while(i<enemy_skips) {
					if(skip[enemy_playerNr][i] == enemy_idx) {
						skipthisship=1;
						break;
					}
					i++;
				}
			}
		}
//		log_add (log_Debug, "SKIP TEST: %i %i\n", enemy_idx, skipthisship);

		if(!skipthisship) {
			ships_left++;
			metric_continue	 = _counter_getBest_getMetric_recursive(my_playerNr, 	my_hShip,	enemy_hShip,	my_skips, 	enemy_skips,	enemy_playerNr, p_metric_enemy,	MAX_SHIPS_PER_SIDE, 0);
			metric_enemy 	 = _counter_getBest_getMetric_recursive(enemy_playerNr,	enemy_hShip,	enemy_hShip,	enemy_skips, 	my_skips,	enemy_playerNr, &metric_my,	MAX_SHIPS_PER_SIDE, 0);

//			log_add (log_Debug, "METRICS: %f %f %f %f\n", metric_continue, *p_metric_enemy, metric_enemy, metric_my);

			metric   	 = _counter_getBest_calcLocalMetric(my_ID, enemy_ID, ship_cost); // metric for current battle
			metric		+=	metric_my*(
							metric_continue > (METRIC_MIDDLE(ship_cost)*2) 	? 
							1 					: 
							metric_continue / (METRIC_MIDDLE(ship_cost)*2)
						);	
				// metric if picking new ship * probability of picking new ship

			metric_continue	*= (((METRIC_MIDDLE(ship_cost)*2) - metric_continue)/(METRIC_MIDDLE(ship_cost)*2)); // * probability that old ship will survive
			metric_continue	 = metric_continue>0 ? metric_continue : 0;			// probablity cannot be less than zero

			metric		+= metric_continue;	// + metric if continue using of old ship * probability that old ship will survive

			if (metric < metric_best || metric_best < -METRIC_ZERO)
			{
				metric_best		= metric;
				metric_enemy_best	= metric_enemy;
//				log_add (log_Debug, "SET: %f %i\n", metric_best, enemy_ID);
			}
//			log_add (log_Debug, "CALC: %f %i %i %i\n", metric, enemy_ID, my_skips, enemy_skips);
		}

		if(lastship)
			break;
	}

//	log_add (log_Debug, "RET: %f %i %i %i\n", metric_best > -METRIC_ZERO ? metric_best : 0, my_skips, enemy_skips, my_ID);
	*p_metric_enemy = metric_enemy_best > -METRIC_ZERO ? metric_enemy_best : METRIC_LOSS;
	return metric_best > -METRIC_ZERO ? metric_best : 0;
}

COUNT
counter_getBest (SIZE my_playerNr)
{
	SIZE enemy_playerNr = !my_playerNr;
	SPECIES_ID enemy_ID, my_ID;
	HELEMENT hObject, hNextObject;
	ELEMENT *ObjectPtr;
	COUNT idx_best, idx_enemy;
	float metric, metric_best, metric_enemy;
	HSTARSHIP my_hShip, my_hNextShip;
	QUEUE *my_ship_q = &race_q[my_playerNr];
	STARSHIP *EnemyShipPtr;

	idx_enemy	=  NO_ID;

	for (hObject = GetHeadElement (); hObject; hObject = hNextObject)
	{
		LockElement (hObject, &ObjectPtr);
		hNextObject = GetSuccElement (ObjectPtr);

		if ((ObjectPtr->state_flags & PLAYER_SHIP) && (ObjectPtr->playerNr == enemy_playerNr))
		{
			GetElementStarShip (ObjectPtr, &EnemyShipPtr);
			
			idx_enemy = EnemyShipPtr->index;
			enemy_ID  = EnemyShipPtr->SpeciesID;
			break;
		}
		
		UnlockElement (hObject);
	}

	metric_best = METRIC_INITIAL;
	for (my_hShip = GetHeadLink (my_ship_q); my_hShip != 0; my_hShip = my_hNextShip)
	{
		STARSHIP *my_StarShipPtr = LockStarShip (my_ship_q, my_hShip);
		my_ID = my_StarShipPtr->SpeciesID;
		my_hNextShip = _GetSuccLink (my_StarShipPtr);
		UnlockStarShip (my_ship_q, my_hShip);

		if(my_ID == NO_ID)
			continue;

		metric = _counter_getBest_getMetric_recursive(my_playerNr, my_hShip, my_hShip/*my_ID, my_StarShipPtr->index*/, 0, 0, my_playerNr, &metric_enemy, idx_enemy, enemy_ID/*, EnemyShipPtr*/);

		if (metric < metric_best || metric_best < -METRIC_ZERO)
		{
			metric_best	= metric;
			idx_best	= my_StarShipPtr->index;
//			log_add (log_Debug, "-> %f\n", metric_best);
		}

//		log_add (log_Debug, "%f\n", metric);
	}

	log_add (log_Debug, "_____SELECTING A SHIP: %i_____\n", idx_best);

	return idx_best;
}


