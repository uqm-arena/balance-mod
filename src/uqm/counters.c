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

#include <math.h>
#include "libs/mathlib.h"
#include "init.h"
#include "counters.h"
#include "libs/log.h"
#include "options.h"
#include "ships/chmmr/chmmr.h"

#define METRIC_INITIAL (-1)
#define METRIC_ZERO (1E-10)
#define METRIC_LOSS (1E+6)
#define METRIC_WIN 0
#define METRIC_MIDDLE(ship_cost) (_counter_getBest_calcLocalMetric(NO_ID, NO_ID, NOT_SPECIAL, NOT_SPECIAL, ship_cost))

#define RATING_KOEFFICIENT_OF_USELESS 0.75

#define METRIC_DEPTH	3
#define RATING_DEPTH	3

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
		0.75,	// SHOFIXTI_ID
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
		0.05,	// CHMMR_ID
		0.20,	// EARTHLING_ID
		0.25,	// ORZ_ID
		0.10,	// PKUNK_ID
		0.50,	// SHOFIXTI_ID
		0.10,	// SPATHI_ID
		0.25,	// SUPOX_ID
		0.20,	// THRADDASH_ID
		0.05,	// UTWIG_ID
		0.15,	// VUX_ID
		0.05,	// YEHAT_ID
		0.15,	// MELNORME_ID
		0.30,	// DRUUGE_ID
		0.20,	// ILWRATH_ID
		0.65,	// MYCON_ID
		0.05,	// SLYLANDRO_ID
		0.35,	// UMGAH_ID
		0.05,	// UR_QUAN_ID
		0.25,	// ZOQFOTPIK_ID
		0.20,	// SYREEN_ID
		0.05,	// KOHR_AH_ID
		0.20,	// ANDROSYNTH_ID
		0.05,	// CHENJESU_ID
		0.05,	// MMRNMHRM_ID
		0.05,	// SIS_SHIP_ID
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
		0.15,	// CHMMR_ID
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

typedef enum {
	NOT_SPECIAL		= 0,
	CHMMR_WITH_2SAT 	= 1,
	CHMMR_WITH_1SAT 	= 2,
	CHMMR_WITH_0SAT 	= 3,
	UTWIG_WITH_NO_ENERGY 	= 4,
	NUM_TABLE_ADDITIONS  	= 5,
} SPECIAL_SPECIES_ID;

static float WINNING_PROBABILITY_TABLE_ADDITION[NUM_TABLE_ADDITIONS][NUM_SPECIES_ID] = {
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
	{ // CHMMR_WITH_2SAT 
		1,	// NO_ID
		0.80,	// ARILOU_ID
		0.40,	// CHMMR_ID
		0.95,	// EARTHLING_ID
		0.40,	// ORZ_ID
		0.70,	// PKUNK_ID
		0.95,	// SHOFIXTI_ID
		0.65,	// SPATHI_ID
		0.95,	// SUPOX_ID
		0.80,	// THRADDASH_ID
		0.30,	// UTWIG_ID
		0.85,	// VUX_ID
		0.65,	// YEHAT_ID
		0.80,	// MELNORME_ID
		0.35,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.85,	// MYCON_ID
		0.70,	// SLYLANDRO_ID
		0.95,	// UMGAH_ID
		0.55,	// UR_QUAN_ID
		0.95,	// ZOQFOTPIK_ID
		0.90,	// SYREEN_ID
		0.55,	// KOHR_AH_ID
		0.65,	// ANDROSYNTH_ID
		0.65,	// CHENJESU_ID
		0.65,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.20,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // CHMMR_WITH_1SAT
		1,	// NO_ID
		0.70,	// ARILOU_ID
		0.35,	// CHMMR_ID
		0.90,	// EARTHLING_ID
		0.35,	// ORZ_ID
		0.65,	// PKUNK_ID
		0.90,	// SHOFIXTI_ID
		0.55,	// SPATHI_ID
		0.70,	// SUPOX_ID
		0.75,	// THRADDASH_ID
		0.40,	// UTWIG_ID
		0.80,	// VUX_ID
		0.65,	// YEHAT_ID
		0.80,	// MELNORME_ID
		0.30,	// DRUUGE_ID
		0.85,	// ILWRATH_ID
		0.80,	// MYCON_ID
		0.65,	// SLYLANDRO_ID
		0.90,	// UMGAH_ID
		0.50,	// UR_QUAN_ID
		0.90,	// ZOQFOTPIK_ID
		0.90,	// SYREEN_ID
		0.55,	// KOHR_AH_ID
		0.50,	// ANDROSYNTH_ID
		0.60,	// CHENJESU_ID
		0.60,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.20,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // CHMMR_WITH_0SAT 
		1,	// NO_ID
		0.50,	// ARILOU_ID
		0.30,	// CHMMR_ID
		0.90,	// EARTHLING_ID
		0.20,	// ORZ_ID
		0.45,	// PKUNK_ID
		0.70,	// SHOFIXTI_ID
		0.50,	// SPATHI_ID
		0.55,	// SUPOX_ID
		0.65,	// THRADDASH_ID
		0.70,	// UTWIG_ID
		0.60,	// VUX_ID
		0.60,	// YEHAT_ID
		0.80,	// MELNORME_ID
		0.25,	// DRUUGE_ID
		0.80,	// ILWRATH_ID
		0.75,	// MYCON_ID
		0.50,	// SLYLANDRO_ID
		0.80,	// UMGAH_ID
		0.25,	// UR_QUAN_ID
		0.80,	// ZOQFOTPIK_ID
		0.80,	// SYREEN_ID
		0.50,	// KOHR_AH_ID
		0.35,	// ANDROSYNTH_ID
		0.45,	// CHENJESU_ID
		0.35,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.20,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
	{ // UTWIG_WITH_NO_ENERGY
		1,	// NO_ID
		0.30,	// ARILOU_ID
		0.20,	// CHMMR_ID
		0.75,	// EARTHLING_ID
		0.15,	// ORZ_ID
		0.50,	// PKUNK_ID
		0.85,	// SHOFIXTI_ID
		0.50,	// SPATHI_ID
		0.45,	// SUPOX_ID
		0.40,	// THRADDASH_ID
		0.25,	// UTWIG_ID
		0.15,	// VUX_ID
		0.55,	// YEHAT_ID
		0.30,	// MELNORME_ID
		0.35,	// DRUUGE_ID
		0.90,	// ILWRATH_ID
		0.85,	// MYCON_ID
		0.55,	// SLYLANDRO_ID
		0.45,	// UMGAH_ID
		0.15,	// UR_QUAN_ID
		0.85,	// ZOQFOTPIK_ID
		0.85,	// SYREEN_ID
		0.35,	// KOHR_AH_ID
		0.55,	// ANDROSYNTH_ID
		0.15,	// CHENJESU_ID
		0.30,	// MMRNMHRM_ID
		0.20,	// SIS_SHIP_ID
		0.70,	// SA_MATRA_ID
		0.50,	// UR_QUAN_PROBE_ID
	},
};

inline float
_counter_getWinningProbability(SPECIES_ID my_ID, SPECIES_ID enemy_ID, SPECIAL_SPECIES_ID my_sID, SPECIAL_SPECIES_ID enemy_sID)
{
	float probability;
	if(my_sID == NOT_SPECIAL && enemy_sID == NOT_SPECIAL)
		probability  = WINNING_PROBABILITY_TABLE[my_ID][enemy_ID];
	else
		if(my_sID == NOT_SPECIAL)
			probability  = (1-WINNING_PROBABILITY_TABLE_ADDITION[enemy_sID][my_ID]);
		else
			probability  = WINNING_PROBABILITY_TABLE_ADDITION[my_sID][enemy_ID];
	// TODO: describe the situation if my_sID != NOT_SPECIAL and enemy_sID != NOT_SPECIAL

	return probability;
}

inline float 
_counter_getBest_calcLocalMetric(SPECIES_ID my_ID, SPECIES_ID enemy_ID, SPECIAL_SPECIES_ID my_sID, SPECIAL_SPECIES_ID enemy_sID, BYTE my_ship_cost)
{
	float metric;
	metric  = 1 / (_counter_getWinningProbability(my_ID, enemy_ID, my_sID, enemy_sID) + METRIC_ZERO);
	metric *= metric;
	metric *= my_ship_cost;
	return metric;
}

float
_counter_getBest_getMetric_recursive(SIZE my_playerNr, HSTARSHIP my_hShip, HSTARSHIP hShip, int my_skips, int enemy_skips, SIZE pick_playerNr, float *p_metric_enemy, COUNT enemy_idx_already, SPECIES_ID enemy_ID_already, float *p_metric_local, char *p_end_reached, SPECIAL_SPECIES_ID *my_ships_sids, SPECIAL_SPECIES_ID *enemy_ships_sids)
{
	int ships_left;
	static COUNT skip[NUM_PLAYERS][MAX_SHIPS_PER_SIDE];
	SIZE enemy_playerNr = !my_playerNr;
	QUEUE *enemy_ship_q = &race_q[enemy_playerNr];
	SPECIES_ID enemy_ID;
	SPECIAL_SPECIES_ID enemy_sID, my_sID;
	HSTARSHIP enemy_hShip, enemy_hNextShip;
	float metric_best, metric, metric_continue, metric_enemy, metric_enemy_r, metric_enemy_best, metric_my, metric_local, metric_local_enemy;

	QUEUE *my_ship_q 	= &race_q[my_playerNr];
	STARSHIP *my_StarShipPtr = LockStarShip (my_ship_q, my_hShip);
	COUNT my_idx 		= my_StarShipPtr->index;
	SPECIES_ID my_ID  	= my_StarShipPtr->SpeciesID;
	BYTE ship_cost 		= my_StarShipPtr->ship_cost;
	UnlockStarShip (my_ship_q, my_hShip);
	my_sID			= my_ships_sids[my_idx];

	QUEUE *ship_q = &race_q[my_playerNr == pick_playerNr ? my_playerNr : enemy_playerNr];
	STARSHIP *StarShipPtr = LockStarShip (ship_q, hShip);
	COUNT idx = StarShipPtr->index;
	UnlockStarShip (ship_q, hShip);

	if(my_playerNr == pick_playerNr)
		skip[my_playerNr	][my_skips++	] = idx;
	else
		skip[enemy_playerNr	][enemy_skips++	] = idx;

	
	if(my_skips >= METRIC_DEPTH || enemy_skips >= METRIC_DEPTH) {
		if(p_metric_local != NULL)
			*p_metric_local = METRIC_MIDDLE(ship_cost);
		*p_metric_enemy = METRIC_MIDDLE(ship_cost);
		*p_end_reached  = 0;
		return METRIC_MIDDLE(ship_cost);
	}

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
		enemy_sID = enemy_ships_sids[enemy_idx];
//		log_add (log_Debug, "SKIP TEST: %i %i\n", enemy_idx, skipthisship);

		if(!skipthisship) {
			ships_left++;
			metric_continue	 = _counter_getBest_getMetric_recursive(my_playerNr, 	my_hShip,	enemy_hShip,	my_skips, 	enemy_skips,	enemy_playerNr, &metric_enemy_r,MAX_SHIPS_PER_SIDE, 0, &metric_local,		p_end_reached,	my_ships_sids, 		enemy_ships_sids);
			metric_enemy 	 = _counter_getBest_getMetric_recursive(enemy_playerNr,	enemy_hShip,	enemy_hShip,	enemy_skips, 	my_skips,	enemy_playerNr, &metric_my,	MAX_SHIPS_PER_SIDE, 0, &metric_local_enemy,	p_end_reached,	enemy_ships_sids,	my_ships_sids);

//			log_add (log_Debug, "METRICS: %i %i %i %f %f %f %f %f %f", my_playerNr, my_idx, enemy_idx, metric_continue, metric_enemy_r, metric_enemy, metric_my, metric_local, METRIC_MIDDLE(ship_cost)*2);

			metric   	 = _counter_getBest_calcLocalMetric(my_ID, enemy_ID, my_sID, enemy_sID, ship_cost); // metric for current battle

//			log_add (log_Debug, "METRICS-S: %f\n", metric);
			skipthisship += metric_my;
			skipthisship += metric_local;
			skipthisship += metric_continue;
			skipthisship += METRIC_MIDDLE(ship_cost);

			metric		+=	metric_my*(
							metric_local > (METRIC_MIDDLE(ship_cost)*2) 	? 
							1 						: 
							metric_local / (METRIC_MIDDLE(ship_cost)*2)
						);	
				// metric if picking new ship * probability of picking new ship

//			log_add (log_Debug, "METRICS-S: %f\n", metric);

			metric_continue	*= (((METRIC_MIDDLE(ship_cost)*2) - metric_local)/(METRIC_MIDDLE(ship_cost)*2)); // * probability that old ship will survive
			metric_continue	 = metric_continue>0 ? metric_continue : 0;			// probablity cannot be less than zero

			metric		+= metric_continue;	// + metric if continue using of old ship * probability that old ship will survive

//			log_add (log_Debug, "METRICS-S: %f\n", metric);

			if (metric < metric_best || metric_best < -METRIC_ZERO)
			{

				if(p_metric_local != NULL)
					*p_metric_local	 = metric;

				skipthisship += metric_enemy;
				skipthisship += metric_local;
				skipthisship += METRIC_MIDDLE(ship_cost);

				metric_enemy_best =	metric_enemy*(

								metric_local > (METRIC_MIDDLE(ship_cost)*2) 	? 
								1 					: 
								metric_local / (METRIC_MIDDLE(ship_cost)*2)
							);	
					// metric if picking new ship * probability of picking new ship

//				log_add (log_Debug, "METRICS-E: %f\n", metric);

				metric_enemy_r	 *= (((METRIC_MIDDLE(ship_cost)*2) - metric_local)/(METRIC_MIDDLE(ship_cost)*2)); // * probability that old ship will survive
				metric_enemy_r	  = metric_enemy_r>0 ? metric_enemy_r : 0;			// probablity cannot be less than zero

				metric_enemy_best+= metric_enemy_r;	// + metric if continue using of old ship * probability that old ship will survive
//				log_add (log_Debug, "METRICS-E: %f\n", metric);

				metric_best	  = metric;
//				log_add (log_Debug, "SET: %f %f %i\n", metric_best, metric_enemy_best, enemy_ID);
			}
//			log_add (log_Debug, "CALC: %f %i %i %i\n", metric, enemy_ID, my_skips, enemy_skips);
		}

		if(lastship)
			break;
	}

	if(metric_best <= -METRIC_ZERO) {
		metric_enemy_best	= METRIC_LOSS;
		metric_best		= METRIC_WIN;
		if(p_metric_local != NULL)
			*p_metric_local = METRIC_WIN;
	}
//	log_add (log_Debug, "RET: %f %i %i %i %f\n", metric_best > -METRIC_ZERO ? metric_best : METRIC_ZERO, my_skips, enemy_skips, my_ID, metric_enemy_best);
	*p_metric_enemy = metric_enemy_best;
	return metric_best;
}

inline int
_counter_getChmmrSatellites(STARSHIP *ChmmrStarShipPtr) {
	HELEMENT hElement, hSuccElement;
	int satellites;
	char onBattleField;

	satellites=0;

	// TODO: don't check all elements if chmmr is not on Battle field;
	onBattleField = 0;
	for (hElement = GetHeadElement (); hElement; hElement = hSuccElement)
	{
		ELEMENT *ElementPtr;
		STARSHIP *StarShipPtr;

		LockElement (hElement, &ElementPtr);
		hSuccElement = GetSuccElement (ElementPtr);
		GetElementStarShip (ElementPtr, &StarShipPtr);
				// Get the STARSHIP that this ELEMENT belongs to.

		if (StarShipPtr == ChmmrStarShipPtr)
		{
			onBattleField=1;

//			if (!(ElementPtr->state_flags & CREW_OBJECT)
//					|| ElementPtr->preprocess_func != crew_preprocess)
			if (ElementPtr->preprocess_func == satellite_preprocess)
				satellites++;
		}

		UnlockElement (hElement);
	}
	
	log_add(log_Debug, "SAT: %i", satellites);

	if(onBattleField)
		return satellites;
	
	if(ChmmrStarShipPtr->flee_counter) {
		int i=0;
		while(i<MISC_STORAGE_SIZE)
			if(((COUNT*)ChmmrStarShipPtr->miscellanea_storage)[i++])
				satellites++;
	} else
		satellites = 3;	// Should be equal to NUM_SATELLITES from chmmr.c

	return satellites;
}

void
_counter_getShipsUsefulness_getShipIDs(SIZE playerNr, SPECIES_ID *ships_ids, SPECIAL_SPECIES_ID *ships_sids, BYTE *ships_costs) {
	QUEUE *ship_q = &race_q[playerNr];
	HSTARSHIP hShip, hNextShip;

	int i=0;
	while(i<MAX_SHIPS_PER_SIDE) {
		ships_ids[i]	  = NO_ID;
		ships_sids[i]	  = NOT_SPECIAL;
		i++;
	}

	for (hShip = GetHeadLink (ship_q); hShip != 0; hShip = hNextShip)
	{
		SPECIES_ID SpeciesID;
		COUNT idx;
		BYTE ship_cost;

		STARSHIP *StarShipPtr = LockStarShip (ship_q, hShip);
		SpeciesID 		 = StarShipPtr->SpeciesID;
		idx			 = StarShipPtr->index;
		ship_cost		 = StarShipPtr->ship_cost;
		hNextShip 		 = _GetSuccLink (StarShipPtr);

		switch(SpeciesID) {
			case CHMMR_ID: {
				int satellites;
				satellites = _counter_getChmmrSatellites(StarShipPtr);
				switch(satellites) {
					case 2:
						ships_sids[idx] = CHMMR_WITH_2SAT;
						break;
					case 1:
						ships_sids[idx] = CHMMR_WITH_1SAT;
						break;
					case 0:
						ships_sids[idx] = CHMMR_WITH_0SAT;
						break;
				}
				break;
			}
			case UTWIG_ID:
				if(!StarShipPtr->RaceDescPtr->ship_info.energy_level)
					ships_sids[idx] = UTWIG_WITH_NO_ENERGY;
				break;
			default:
				break;
		}

		UnlockStarShip (ship_q, hShip);

		ships_ids[idx]	 = SpeciesID;
		ships_costs[idx] = ship_cost;
		log_add (log_Debug, "SIDS: %i %i", idx, ships_sids[idx]);
	}

	return;
}

float *
_counter_getShipsUsefulness(int depth, SPECIES_ID *my_ships_ids, SPECIES_ID *enemy_ships_ids, SPECIAL_SPECIES_ID *my_ships_sids, SPECIAL_SPECIES_ID *enemy_ships_sids, BYTE *my_ships_costs, BYTE *enemy_ships_costs, float *enemyshipsproblemness)
{
	static float shipsusefulness[MAX_SHIPS_PER_SIDE];
	COUNT countering[MAX_SHIPS_PER_SIDE];
	int i;
//	SIZE enemy_playerNr 	= !my_playerNr;

	i=0;
	while(i<MAX_SHIPS_PER_SIDE) 
		countering[i++]	  = MAX_SHIPS_PER_SIDE;

	char oneloopmore;
	do {
		oneloopmore=0;
		int my_idx=0;
		while(my_idx < MAX_SHIPS_PER_SIDE) {
			SPECIES_ID my_ID;
			SPECIAL_SPECIES_ID my_sID;
			float shipusefulness;
			BYTE my_ship_cost;
			my_ID 		= my_ships_ids[my_idx];
			my_sID		= my_ships_sids[my_idx];
			my_ship_cost	= my_ships_costs[my_idx];

			if((my_ID == NO_ID) || (countering[my_idx] != MAX_SHIPS_PER_SIDE)) {
				my_idx++;
				continue;
			}

			shipusefulness = 0;

			int enemy_idx;
			enemy_idx=0;

			while(enemy_idx < MAX_SHIPS_PER_SIDE) {
				float shipusefulness_part;
				BYTE enemy_ship_cost;
				SPECIES_ID enemy_ID;
				SPECIAL_SPECIES_ID enemy_sID;
				enemy_ID	= enemy_ships_ids[enemy_idx];
				enemy_sID	= enemy_ships_sids[enemy_idx];
				enemy_ship_cost	= enemy_ships_costs[enemy_idx];

				if(enemy_ID == NO_ID) {
					enemy_idx++;
					continue;
				}

				shipusefulness_part  = enemyshipsproblemness[enemy_idx] * _counter_getWinningProbability(my_ID, enemy_ID, my_sID, enemy_sID);

				if(depth) {
					shipusefulness_part *= shipusefulness_part;
					shipusefulness      += shipusefulness_part * enemy_ship_cost/my_ship_cost;
				} else {
					if(shipusefulness_part > shipusefulness*(1+1E-10)) {
						int i;
						i=0;
						while(i<MAX_SHIPS_PER_SIDE) {
							if((my_ships_ids[i] == NO_ID) || (i == my_idx)) {
								i++;
								continue;
							}

							if(countering[i] == enemy_idx) {
								if(shipusefulness_part >= shipsusefulness[i]*(1+1E-10)) {
									shipsusefulness[i] = shipsusefulness[i] * RATING_KOEFFICIENT_OF_USELESS;
									countering[i]      = MAX_SHIPS_PER_SIDE;
									oneloopmore |= 1;
								} else
									break;
							}
							i++;
						}
						if(i == MAX_SHIPS_PER_SIDE) {
							countering[my_idx] = enemy_idx;
							oneloopmore |= 2;
						} 
						shipusefulness = shipusefulness_part;
					}
				}

				enemy_idx++;
			}

			if(depth) {
				shipsusefulness[my_idx] = sqrt(shipusefulness);
			} else 
				shipsusefulness[my_idx] = 
					(countering[my_idx] == MAX_SHIPS_PER_SIDE  ?  RATING_KOEFFICIENT_OF_USELESS  :  1)
							* shipusefulness;

			log_add (log_Debug, "%i\t%i\t%f\t%i\t%f", depth, my_idx, shipsusefulness[my_idx], countering[my_idx], (countering[my_idx] == MAX_SHIPS_PER_SIDE  ?  RATING_KOEFFICIENT_OF_USELESS  :  1));
			assert(!isinf(shipsusefulness[my_idx]));
			my_idx++;
		}
	} while(oneloopmore == (1|2));

/*
	if(!depth) {
		i=0;
		while(i<MAX_SHIPS_PER_SIDE) 
			printf("%i ", countering[i++]);
		printf("\n");
	}
*/

	return shipsusefulness;
}

float *
counter_getShipsUsefulness(SIZE my_playerNr, int depth, SPECIAL_SPECIES_ID *my_ships_sids, SPECIAL_SPECIES_ID *enemy_ships_sids)
{
	static float shipsusefulness[MAX_SHIPS_PER_SIDE], *shipsusefulness_t, *enemyshipsproblemness;
	SPECIES_ID my_ships_ids[MAX_SHIPS_PER_SIDE], enemy_ships_ids[MAX_SHIPS_PER_SIDE];
	BYTE my_ships_costs[MAX_SHIPS_PER_SIDE], enemy_ships_costs[MAX_SHIPS_PER_SIDE];
	int i;

	SIZE enemy_playerNr 	= !my_playerNr;

	_counter_getShipsUsefulness_getShipIDs(my_playerNr, 	my_ships_ids, 		my_ships_sids,		my_ships_costs);
	_counter_getShipsUsefulness_getShipIDs(enemy_playerNr, 	enemy_ships_ids,	enemy_ships_sids,	enemy_ships_costs);

	i=0;
	while(i<MAX_SHIPS_PER_SIDE) {
		shipsusefulness[i] = my_ships_costs[i];
		i++;
	}

	while(depth>0) {
		depth--;
		enemyshipsproblemness 	= _counter_getShipsUsefulness(depth, enemy_ships_ids, my_ships_ids, enemy_ships_sids, my_ships_sids, enemy_ships_costs, my_ships_costs, shipsusefulness);
		shipsusefulness_t	= _counter_getShipsUsefulness(depth, my_ships_ids, enemy_ships_ids, my_ships_sids, enemy_ships_sids, my_ships_costs, enemy_ships_costs, enemyshipsproblemness);
		memcpy(shipsusefulness, shipsusefulness_t, sizeof(shipsusefulness));
	}

	return shipsusefulness;
}

static int counter_getBest_count;

COUNT
counter_getBest (SIZE my_playerNr)
{
	SIZE enemy_playerNr = !my_playerNr;
	SPECIES_ID enemy_ID, my_ID;
	HELEMENT hObject, hNextObject;
	ELEMENT *ObjectPtr;
	COUNT idx_best, idx_enemy;
	float *usefulness;
	float metric, metric_best, metric_enemy, metric_local;
	HSTARSHIP my_hShip, my_hNextShip;
	QUEUE *my_ship_q = &race_q[my_playerNr];
	STARSHIP *EnemyShipPtr;
	SPECIAL_SPECIES_ID my_ships_sids[MAX_SHIPS_PER_SIDE], enemy_ships_sids[MAX_SHIPS_PER_SIDE];

	counter_getBest_count++;

	idx_enemy	= MAX_SHIPS_PER_SIDE;
	enemy_ID	= NO_ID;

	for (hObject = GetHeadElement (); hObject; hObject = hNextObject)
	{
		LockElement (hObject, &ObjectPtr);
		hNextObject = GetSuccElement (ObjectPtr);

		if ((ObjectPtr->state_flags & PLAYER_SHIP) && (ObjectPtr->playerNr == enemy_playerNr))
		{
			GetElementStarShip (ObjectPtr, &EnemyShipPtr);
			
			if(!IS_RETREAT(EnemyShipPtr)) {
				idx_enemy = EnemyShipPtr->index;
				enemy_ID  = EnemyShipPtr->SpeciesID;
			}
			break;
		}
		
		UnlockElement (hObject);
	}

	usefulness = counter_getShipsUsefulness(my_playerNr, RATING_DEPTH, my_ships_sids, enemy_ships_sids);

	metric_best = METRIC_INITIAL;
	for (my_hShip = GetHeadLink (my_ship_q); my_hShip != 0; my_hShip = my_hNextShip)
	{
		char end_reached;

		STARSHIP *my_StarShipPtr = LockStarShip (my_ship_q, my_hShip);
		my_ID = my_StarShipPtr->SpeciesID;
		my_hNextShip = _GetSuccLink (my_StarShipPtr);
		UnlockStarShip (my_ship_q, my_hShip);

		if(my_ID == NO_ID)
			continue;

		if(counter_getBest_count == 1 /* the first pick */) {
			end_reached=1;
			metric = usefulness[my_StarShipPtr->index] * my_StarShipPtr->ship_cost * my_StarShipPtr->ship_cost;
		} else { // Not the first pick
			end_reached=1;
			metric = _counter_getBest_getMetric_recursive(my_playerNr, my_hShip, my_hShip/*my_ID, my_StarShipPtr->index*/, 0, 0, my_playerNr, &metric_enemy, idx_enemy, enemy_ID/*, EnemyShipPtr*/, &metric_local, &end_reached, my_ships_sids, enemy_ships_sids);
		}

		if(!end_reached) { // selecting from few good variants if end of ships is not reached while calculating the metric
			float k, metric_k, rnd;
			int percents;
			k = sqrt(sqrt(usefulness[my_StarShipPtr->index]/metric_local));

			metric_k = metric*k;

			// Random is for humanlikely behaviour

			rnd 	 = (float)(TFB_Random()%2001);		// 0		  .. 2 000	    [evenly]
			rnd	-= 1000;				// -1 000         .. 1 000	    [evenly]
			log_add (log_Debug, "RND: %f", rnd);
			rnd	 = rnd*rnd*rnd;				// -1 000 000 000 .. 1 000 000 000  [centered]
			rnd	/= 1000*1000;				// -1 000         .. 1 000	    [centered]
			rnd	 = rnd*rnd*rnd;				// -1 000 000 000 .. 1 000 000 000  [2x centered]
			rnd	/= 1000*1000*1000;			// -1             .. 1		    [2x centered]
			rnd	 = (rnd+1)*50;				//  0		  .. 100	    [2x centered to "50"]
			percents = rnd;
			log_add (log_Debug, "%i %f %f %i (%f)", my_ID, metric, metric_k, percents, rnd);

			metric   = (metric*(100-percents))/100 + (metric_k*percents)/100;
			log_add (log_Debug, "R: %f", metric);
		}

		if (metric < metric_best || metric_best < -METRIC_ZERO)
		{
			metric_best	= metric;
			idx_best	= my_StarShipPtr->index;
			log_add (log_Debug, "-> %f", metric_best);
		}

		log_add (log_Debug, "M: %i %f", my_StarShipPtr->index, metric);
	}

	log_add (log_Debug, "_____SELECTING A SHIP: %i_____\n", idx_best);

	return idx_best;
}

char
counter_shouldRunAway(STARSHIP *StarShipPtr) {
	return 1;
}

void counter_init() {
	counter_getBest_count=0;
}

