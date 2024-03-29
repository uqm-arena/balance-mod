//Copyright Paul Reiche, Fred Ford. 1992-2002

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

#include "../ship.h"
#include "yehat.h"
#include "resinst.h"
#include "libs/mathlib.h"

// Core Characteristics
#define MAX_CREW 20
#define MAX_ENERGY 10
#define ENERGY_REGENERATION 2
#define ENERGY_WAIT 6
#define MAX_THRUST 32
#define THRUST_INCREMENT 8
#define THRUST_WAIT 3
#define TURN_WAIT 2
#define SHIP_MASS 3

// Twin Pulse Cannons
#define WEAPON_ENERGY_COST 1
#define WEAPON_WAIT 0
#define YEHAT_OFFSET 16
#define LAUNCH_OFFS 32
#define MISSILE_SPEED 80
#define MISSILE_LIFE 10
#define MISSILE_HITS 1
#define MISSILE_DAMAGE 1
#define MISSILE_OFFSET 1

// Shield
#define SPECIAL_ENERGY_COST 2
#define SPECIAL_WAIT 7 // Shield duration
#define SPECIAL_WAIT_BONUS 0 // Add this to the shield's duration at the start of a 'fresh' shield activation
#define ENERGY_WAIT_EXTRA 8 // How long energy recovery stalls after shields drop

static RACE_DESC yehat_desc =
{
	{ /* SHIP_INFO */
		FIRES_FORE | SHIELD_DEFENSE,
		18, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		YEHAT_RACE_STRINGS,
		YEHAT_ICON_MASK_PMAP_ANIM,
		YEHAT_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		750 / SPHERE_RADIUS_INCREMENT * 2, /* Initial SoI radius */
		{ /* Known location (center of SoI) */
			4970, 40,
		},
	},
	{
		MAX_THRUST,
		THRUST_INCREMENT,
		ENERGY_REGENERATION,
		WEAPON_ENERGY_COST,
		SPECIAL_ENERGY_COST,
		ENERGY_WAIT,
		TURN_WAIT,
		THRUST_WAIT,
		WEAPON_WAIT,
		SPECIAL_WAIT,
		SHIP_MASS,
	},
	{
		{
			YEHAT_BIG_MASK_PMAP_ANIM,
			YEHAT_MED_MASK_PMAP_ANIM,
			YEHAT_SML_MASK_PMAP_ANIM,
		},
		{
			YEHAT_CANNON_BIG_MASK_PMAP_ANIM,
			YEHAT_CANNON_MED_MASK_PMAP_ANIM,
			YEHAT_CANNON_SML_MASK_PMAP_ANIM,
		},
		{
			SHIELD_BIG_MASK_ANIM,
			SHIELD_MED_MASK_ANIM,
			SHIELD_SML_MASK_ANIM,
		},
		{
			YEHAT_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		YEHAT_VICTORY_SONG,
		YEHAT_SHIP_SOUNDS,
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		NULL, NULL
	},
	{
		0,
		MISSILE_SPEED * MISSILE_LIFE / 3,
		NULL,
	},
	(UNINIT_FUNC *) NULL,
	(PREPROCESS_FUNC *) NULL,
	(POSTPROCESS_FUNC *) NULL,
	(INIT_WEAPON_FUNC *) NULL,
	0,
	0, /* CodeRef */
};

static COUNT
initialize_standard_missiles (ELEMENT *ShipPtr, HELEMENT MissileArray[])
{
    COUNT i;
	SIZE offs_x, offs_y;
	STARSHIP *StarShipPtr;
	MISSILE_BLOCK MissileBlock;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.face = MissileBlock.index = StarShipPtr->ShipFacing;
	MissileBlock.sender = ShipPtr->playerNr;
	MissileBlock.flags = IGNORE_SIMILAR;
	MissileBlock.pixoffs = YEHAT_OFFSET;
	MissileBlock.speed = MISSILE_SPEED;
	MissileBlock.hit_points = MISSILE_HITS;
	MissileBlock.damage = MISSILE_DAMAGE;
	MissileBlock.life = MISSILE_LIFE;
	MissileBlock.preprocess_func = NULL;
	MissileBlock.blast_offs = MISSILE_OFFSET;

	offs_x = -SINE (FACING_TO_ANGLE (MissileBlock.face), LAUNCH_OFFS);
	offs_y = COSINE (FACING_TO_ANGLE (MissileBlock.face), LAUNCH_OFFS);

	for(i = 0; i < 2; ++i)
	{
		if (i == 0)
		{
			MissileBlock.cx = ShipPtr->next.location.x + offs_x;
			MissileBlock.cy = ShipPtr->next.location.y + offs_y;
		}
		else
		{
			MissileBlock.cx = ShipPtr->next.location.x - offs_x;
			MissileBlock.cy = ShipPtr->next.location.y - offs_y;
		}
	
		if ((MissileArray[i] = initialize_missile (&MissileBlock)))
		{		
			SIZE dx, dy;
			ELEMENT *MissilePtr;

			LockElement (MissileArray[i], &MissilePtr);

			GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
			dx = dx * 3/4;
			dy = dy * 3/4;

			// Add some of the Terminator's velocity to its projectiles
			DeltaVelocityComponents (&MissilePtr->velocity, dx, dy);
			MissilePtr->current.location.x -= VELOCITY_TO_WORLD (dx);
			MissilePtr->current.location.y -= VELOCITY_TO_WORLD (dy);

			UnlockElement (MissileArray[i]);
		}
	}

	return (2);
}

static void
yehat_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern,
		COUNT ConcernCounter)
{
	SIZE ShieldStatus;
	STARSHIP *StarShipPtr;
	EVALUATE_DESC *lpEvalDesc;

	ShieldStatus = -1;
	lpEvalDesc = &ObjectsOfConcern[ENEMY_WEAPON_INDEX];
	if (lpEvalDesc->ObjectPtr && lpEvalDesc->MoveState == ENTICE)
	{
		ShieldStatus = 0;
		if (!(lpEvalDesc->ObjectPtr->state_flags & (FINITE_LIFE | CREW_OBJECT)))
			lpEvalDesc->MoveState = PURSUE;
		else if (lpEvalDesc->ObjectPtr->mass_points
				|| (lpEvalDesc->ObjectPtr->state_flags & CREW_OBJECT))
		{
			if (!(lpEvalDesc->ObjectPtr->state_flags & FINITE_LIFE))
				lpEvalDesc->which_turn <<= 1;
			else
			{
				if ((lpEvalDesc->which_turn >>= 1) == 0)
					lpEvalDesc->which_turn = 1;

				if (lpEvalDesc->ObjectPtr->mass_points)
					lpEvalDesc->ObjectPtr = 0;
				else
					lpEvalDesc->MoveState = PURSUE;
			}
			ShieldStatus = 1;
		}
	}

	GetElementStarShip (ShipPtr, &StarShipPtr);
	if (StarShipPtr->special_counter == 0)
	{
		StarShipPtr->ship_input_state &= ~SPECIAL;
		if (ShieldStatus)
		{
			if (ShipPtr->life_span <= NORMAL_LIFE + 1
					&& (ShieldStatus > 0 || lpEvalDesc->ObjectPtr)
						&& lpEvalDesc->which_turn <= 2
						&& (ShieldStatus > 0
							|| (lpEvalDesc->ObjectPtr->state_flags & PLAYER_SHIP) /* means IMMEDIATE WEAPON */
							|| PlotIntercept (lpEvalDesc->ObjectPtr, ShipPtr, 2, 0))
					&& (TFB_Random () & 4))
				StarShipPtr->ship_input_state |= SPECIAL;

			if (lpEvalDesc->ObjectPtr
					&& !(lpEvalDesc->ObjectPtr->state_flags & CREW_OBJECT))
				lpEvalDesc->ObjectPtr = 0;
		}
	}

	if ((lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX])->ObjectPtr)
	{
		STARSHIP *EnemyStarShipPtr;

		GetElementStarShip (lpEvalDesc->ObjectPtr, &EnemyStarShipPtr);
		if (!(EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags
				& IMMEDIATE_WEAPON))
			lpEvalDesc->MoveState = PURSUE;
	}
	ship_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter);
/*
	if (StarShipPtr->RaceDescPtr->ship_info.energy_level <= SPECIAL_ENERGY_COST)
		StarShipPtr->ship_input_state &= ~WEAPON;
*/
}

static void
yehat_postprocess (ELEMENT *ElementPtr)
{
	if (!(ElementPtr->state_flags & NONSOLID))
	{
		STARSHIP *StarShipPtr;

		GetElementStarShip (ElementPtr, &StarShipPtr);
				/* take care of shield effect */
#ifdef OLD
		SetPrimColor (
				&(GLOBAL (DisplayArray))[ElementPtr->PrimIndex],
				BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x1F), 0x0F)
				);
		SetPrimType (
				&(GLOBAL (DisplayArray))[ElementPtr->PrimIndex],
				STAMPFILL_PRIM
				);
#endif /* OLD */
	}

#ifdef OLD
		if (ElementPtr->life_span > NORMAL_LIFE)
		{
			HELEMENT hShipElement;

		if (hShipElement = AllocElement ())
		{
			ELEMENTPTR ShipElementPtr;

			InsertElement (hShipElement, GetSuccElement (ElementPtr));
			LockElement (hShipElement, &ShipElementPtr);

			ShipElementPtr->state_flags =
						/* in place of APPEARING */
					(CHANGING | PRE_PROCESS | POST_PROCESS)
					| FINITE_LIFE | NONSOLID
					| (ElementPtr->state_flags & (GOOD_GUY | BAD_GUY));
			SetPrimType (
					&(GLOBAL (DisplayArray))[ShipElementPtr->PrimIndex],
					STAMP_PRIM
					);

			ShipElementPtr->life_span = 0; /* because preprocessing
												 * will not be done
												 */
			ShipElementPtr->current.location = ElementPtr->next.location;
			ShipElementPtr->current.image.farray = StarShipPtr->RaceDescPtr->ship_data.ship;
			ShipElementPtr->current.image.frame =
					SetAbsFrameIndex (StarShipPtr->RaceDescPtr->ship_data.ship[0],
					StarShipPtr->ShipFacing);
			ShipElementPtr->next = ShipElementPtr->current;
			ShipElementPtr->preprocess_func =
					ShipElementPtr->postprocess_func =
					ShipElementPtr->death_func = NULL_PTR;
			ZeroVelocityComponents (&ShipElementPtr->velocity);

			UnlockElement (hShipElement);
		}
	}
#endif /* OLD */
}

static void
yehat_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);

	if (!(ElementPtr->state_flags & APPEARING))
	{
		if ((ElementPtr->life_span > NORMAL_LIFE
				/* take care of shield effect */
				&& --ElementPtr->life_span == NORMAL_LIFE)
				|| (ElementPtr->life_span == NORMAL_LIFE
				&& ElementPtr->next.image.farray
						== StarShipPtr->RaceDescPtr->ship_data.special))
		{
#ifdef NEVER
			SetPrimType (
					&(GLOBAL (DisplayArray))[ElementPtr->PrimIndex],
					STAMP_PRIM
					);
#endif /* NEVER */

			ElementPtr->next.image.farray = StarShipPtr->RaceDescPtr->ship_data.ship;
			ElementPtr->next.image.frame =
					SetEquFrameIndex (StarShipPtr->RaceDescPtr->ship_data.ship[0],
					ElementPtr->next.image.frame);
			ElementPtr->state_flags |= CHANGING;
		}

		if ((StarShipPtr->cur_status_flags & SPECIAL)
			&& StarShipPtr->special_counter == 0)
		{
			if (StarShipPtr->RaceDescPtr->ship_info.energy_level < SPECIAL_ENERGY_COST)
				DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST); /* so text will flash */
			else
			{				
				// Only play the sound effect if the shield was off previously
				if (ElementPtr->current.image.farray != StarShipPtr->RaceDescPtr->ship_data.special)
				{
					ProcessSound (SetAbsSoundIndex (
						/* YEHAT_SHIELD_ON */
						StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);
				}
				
				ElementPtr->next.image.farray = StarShipPtr->RaceDescPtr->ship_data.special;
				ElementPtr->next.image.frame =
					SetEquFrameIndex (StarShipPtr->RaceDescPtr->ship_data.special[0],
					ElementPtr->next.image.frame);
				ElementPtr->state_flags |= CHANGING;
				
				DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST);

				ElementPtr->life_span = SPECIAL_WAIT + NORMAL_LIFE;
				StarShipPtr->special_counter = SPECIAL_WAIT;

				// Extend shield duration if this is the start of a 'fresh' shield activation
				if (StarShipPtr->static_counter == 0)
				{
					ElementPtr->life_span += SPECIAL_WAIT_BONUS;
					StarShipPtr->special_counter += SPECIAL_WAIT_BONUS;
				}
				
				// Shield activation will delay the next battery recharge. Effect is non-cumulative.
				StarShipPtr->static_counter = ENERGY_WAIT_EXTRA;
			}
		}
	}

	// Stall battery regen while shield is up
	if (ElementPtr->life_span > NORMAL_LIFE)
		++StarShipPtr->energy_counter;

	// Dump static_counter into energy_counter when the normal delay elapses
	if (StarShipPtr->energy_counter == 0
			&& StarShipPtr->static_counter)
	{
		StarShipPtr->energy_counter += StarShipPtr->static_counter;
		StarShipPtr->static_counter = 0;
	}
}

RACE_DESC*
init_yehat (void)
{
	RACE_DESC *RaceDescPtr;

	yehat_desc.preprocess_func = yehat_preprocess;
	yehat_desc.postprocess_func = yehat_postprocess;
	yehat_desc.init_weapon_func = initialize_standard_missiles;
	yehat_desc.cyborg_control.intelligence_func = yehat_intelligence;

	RaceDescPtr = &yehat_desc;

	return (RaceDescPtr);
}
