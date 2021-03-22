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
#include "ilwrath.h"
#include "resinst.h"
#include "uqm/colors.h"
#include "uqm/globdata.h"
#include "uqm/setup.h"
#include "libs/mathlib.h"

// Core Characteristics
#define MAX_CREW 22
#define MAX_ENERGY 12
#define ENERGY_REGENERATION 4
#define ENERGY_WAIT 6
#define MAX_THRUST 26
#define THRUST_INCREMENT 5
#define THRUST_WAIT 0
#define TURN_WAIT 2
#define SHIP_MASS 7

// Hellfire Spout
#define WEAPON_ENERGY_COST 1
#define WEAPON_WAIT 0
#define ALTFIRE_ENERGY_COST 2
#define ALTFIRE_WAIT 1
#define ALTFIRE_THRESHOLD 4
#define ILWRATH_OFFSET 29
#define MISSILE_SPEED 32
#define MISSILE_LIFE 8
#define MISSILE_HITS 1
#define MISSILE_DAMAGE 1
#define MISSILE_OFFSET 0

// Cloaking Device
#define SPECIAL_ENERGY_COST 2
#define SPECIAL_WAIT 13

static RACE_DESC ilwrath_desc =
{
	{ /* SHIP_INFO */
		FIRES_FORE,
		11, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		ILWRATH_RACE_STRINGS,
		ILWRATH_ICON_MASK_PMAP_ANIM,
		ILWRATH_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		1410 / SPHERE_RADIUS_INCREMENT * 2, /* Initial SoI radius */
		{ /* Known location (center of SoI) */
			48, 1700,
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
			ILWRATH_BIG_MASK_PMAP_ANIM,
			ILWRATH_MED_MASK_PMAP_ANIM,
			ILWRATH_SML_MASK_PMAP_ANIM,
		},
		{
			FIRE_BIG_MASK_PMAP_ANIM,
			FIRE_MED_MASK_PMAP_ANIM,
			FIRE_SML_MASK_PMAP_ANIM,
		},
		{
			NULL_RESOURCE,
			NULL_RESOURCE,
			NULL_RESOURCE,
		},
		{
			ILWRATH_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		ILWRATH_VICTORY_SONG,
		ILWRATH_SHIP_SOUNDS,
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		NULL, NULL
	},
	{
		0,
		CLOSE_RANGE_WEAPON,
		NULL,
	},
	(UNINIT_FUNC *) NULL,
	(PREPROCESS_FUNC *) NULL,
	(POSTPROCESS_FUNC *) NULL,
	(INIT_WEAPON_FUNC *) NULL,
	0,
	0, /* CodeRef */
};

static void
flame_preprocess (ELEMENT *ElementPtr)
{
	if (ElementPtr->turn_wait > 0)
		--ElementPtr->turn_wait;
	else
	{
		ElementPtr->next.image.frame =
				IncFrameIndex (ElementPtr->current.image.frame);
		ElementPtr->state_flags |= CHANGING;

		ElementPtr->turn_wait = ElementPtr->next_turn;
	}
}

static void
flame_collision (ELEMENT *ElementPtr0, POINT *pPt0, ELEMENT *ElementPtr1, POINT *pPt1)
{
	weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
	ElementPtr0->state_flags &= ~DISAPPEARING;
	ElementPtr0->state_flags |= NONSOLID;
}

static COUNT
initialize_flame (ELEMENT *ShipPtr, HELEMENT FlameArray[])
{
	STARSHIP *StarShipPtr;
	MISSILE_BLOCK MissileBlock;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	MissileBlock.cx = ShipPtr->next.location.x;
	MissileBlock.cy = ShipPtr->next.location.y;
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.face = StarShipPtr->ShipFacing;
	MissileBlock.index = 0;
	MissileBlock.sender = ShipPtr->playerNr;
	MissileBlock.flags = IGNORE_SIMILAR;
	MissileBlock.pixoffs = ILWRATH_OFFSET;
	MissileBlock.speed = MISSILE_SPEED;
	MissileBlock.hit_points = MISSILE_HITS;
	MissileBlock.damage = MISSILE_DAMAGE;
	MissileBlock.life = MISSILE_LIFE;
	MissileBlock.preprocess_func = flame_preprocess;
	MissileBlock.blast_offs = MISSILE_OFFSET;
	FlameArray[0] = initialize_missile (&MissileBlock);

	if (FlameArray[0])
	{
		SIZE dx, dy;
		ELEMENT *FlamePtr;

		LockElement (FlameArray[0], &FlamePtr);
		GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
		DeltaVelocityComponents (&FlamePtr->velocity, dx, dy);
		FlamePtr->current.location.x -= VELOCITY_TO_WORLD (dx);
		FlamePtr->current.location.y -= VELOCITY_TO_WORLD (dy);

		FlamePtr->collision_func = flame_collision;
		FlamePtr->turn_wait = 0;
		UnlockElement (FlameArray[0]);
	}

	return (1);
}

static void
initialize_diagonal_flame (ELEMENT *ElementPtr)
{
	COUNT i;
	STARSHIP *StarShipPtr;
	MISSILE_BLOCK MissileBlock;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.index = 0;
	MissileBlock.sender = ElementPtr->playerNr;
	MissileBlock.flags = IGNORE_SIMILAR;
	MissileBlock.pixoffs = ILWRATH_OFFSET;
	MissileBlock.speed = MISSILE_SPEED;
	MissileBlock.hit_points = MISSILE_HITS;
	MissileBlock.damage = MISSILE_DAMAGE;
	MissileBlock.life = MISSILE_LIFE;
	MissileBlock.preprocess_func = flame_preprocess;
	MissileBlock.blast_offs = MISSILE_OFFSET;

	for(i = 0; i < 2; ++i)
	{
		HELEMENT hFlame;
	
		if (i == 0)
		{
			MissileBlock.cx = ElementPtr->next.location.x
				+ COSINE(FACING_TO_ANGLE(StarShipPtr->ShipFacing + 4), -32);
			MissileBlock.cy = ElementPtr->next.location.y
				+ SINE(FACING_TO_ANGLE(StarShipPtr->ShipFacing + 4), -32);

			MissileBlock.face = NORMALIZE_FACING (StarShipPtr->ShipFacing + 2);
		}
		else
		{
			MissileBlock.cx = ElementPtr->next.location.x
				+ COSINE(FACING_TO_ANGLE(StarShipPtr->ShipFacing + 4), 32);
			MissileBlock.cy = ElementPtr->next.location.y
				+ SINE(FACING_TO_ANGLE(StarShipPtr->ShipFacing + 4), 32);

			MissileBlock.face = NORMALIZE_FACING (StarShipPtr->ShipFacing - 2);
		}

		hFlame = initialize_missile (&MissileBlock);
		if (hFlame)
		{
			SIZE dx, dy;
			ELEMENT *FlamePtr;
	
			LockElement (hFlame, &FlamePtr);
			SetElementStarShip (FlamePtr, StarShipPtr);
			FlamePtr->hTarget = 0;
			GetCurrentVelocityComponents (&ElementPtr->velocity, &dx, &dy);
			DeltaVelocityComponents (&FlamePtr->velocity, dx, dy);
			FlamePtr->current.location.x -= VELOCITY_TO_WORLD (dx);
			FlamePtr->current.location.y -= VELOCITY_TO_WORLD (dy);
	
			FlamePtr->collision_func = flame_collision;
			FlamePtr->turn_wait = 0;
			UnlockElement (hFlame);
			PutElement (hFlame);
		}
	}
}

static void
ilwrath_postprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;
    
	GetElementStarShip (ElementPtr, &StarShipPtr);
	
	if ((StarShipPtr->cur_status_flags & WEAPON)
			&& StarShipPtr->auxiliary_counter == 0
			&& StarShipPtr->RaceDescPtr->ship_info.energy_level >= ALTFIRE_THRESHOLD)
	{
		initialize_diagonal_flame (ElementPtr);
		
		DeltaEnergy (ElementPtr, -ALTFIRE_ENERGY_COST);
		StarShipPtr->auxiliary_counter += ALTFIRE_WAIT + 1;
	}
}

static void
ilwrath_preprocess (ELEMENT *ElementPtr)
{
	STATUS_FLAGS status_flags;
	STARSHIP *StarShipPtr;
	PRIMITIVE *lpPrim;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	status_flags = StarShipPtr->cur_status_flags;
	lpPrim = &(GLOBAL (DisplayArray))[ElementPtr->PrimIndex];
	if (GetPrimType (lpPrim) == STAMPFILL_PRIM)
	{
		Color color;
		BOOLEAN weapon_discharge;

		color = GetPrimColor (lpPrim);
		weapon_discharge = ((status_flags & WEAPON)
				&& StarShipPtr->RaceDescPtr->ship_info.energy_level >= WEAPON_ENERGY_COST);
	
		if (
			weapon_discharge
			|| (
				StarShipPtr->special_counter == 0
				&& ( 
					((!sameColor(color, BLACK_COLOR)) && (!sameColor(color, INVIS_COLOR)))
					|| status_flags & SPECIAL
				)
			)
		)
		{
			if (sameColor (color,
					BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x1F), 0x0F)))
				SetPrimType (lpPrim, STAMP_PRIM);
			else if (sameColor (color,
					BUILD_COLOR (MAKE_RGB15 (0x0A, 0x1F, 0x1F), 0x0B)))
				SetPrimColor (lpPrim,
						BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x1F), 0x0F));
			else if (sameColor (color,
					BUILD_COLOR (MAKE_RGB15 (0x00, 0x14, 0x14), 0x03)))
				SetPrimColor (lpPrim,
						BUILD_COLOR (MAKE_RGB15 (0x0A, 0x1F, 0x1F), 0x0B));
			else if (sameColor (color,
					BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x1F), 0x09)))
				SetPrimColor (lpPrim,
						BUILD_COLOR (MAKE_RGB15 (0x00, 0x14, 0x14), 0x03));
			else if (sameColor (color,
					BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x14), 0x01)))
				SetPrimColor (lpPrim,
						BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x1F), 0x09));
			else
			{
				ProcessSound (SetAbsSoundIndex (
						/* CLOAKING_OFF */
					StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 2), ElementPtr);
			
				SetPrimColor (lpPrim, BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x14), 0x01));
			}
			
			ElementPtr->state_flags |= CHANGING;
			status_flags &= ~SPECIAL;
			StarShipPtr->special_counter = 0;
		}
		else if ( (!sameColor (color, BLACK_COLOR))
				&& (!sameColor (color, INVIS_COLOR)))
		{
			if (sameColor (color, BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x14), 0x01)))
			{
				if (PlayerControl[ElementPtr->playerNr] & HUMAN_CONTROL)
					SetPrimColor (lpPrim, INVIS_COLOR);
				else
					SetPrimColor (lpPrim, BLACK_COLOR);
				Untarget (ElementPtr);
			}
			else if (sameColor (color,
					BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x1F), 0x09)))
				SetPrimColor (lpPrim,
						BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x14), 0x01));
			else if (sameColor (color,
					BUILD_COLOR (MAKE_RGB15 (0x00, 0x14, 0x14), 0x03)))
				SetPrimColor (lpPrim,
						BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x1F), 0x09));
			else if (sameColor (color,
					BUILD_COLOR (MAKE_RGB15 (0x0A, 0x1F, 0x1F), 0x0B)))
				SetPrimColor (lpPrim,
						BUILD_COLOR (MAKE_RGB15 (0x00, 0x14, 0x14), 0x03));
			else
				SetPrimColor (lpPrim,
						BUILD_COLOR (MAKE_RGB15 (0x0A, 0x1F, 0x1F), 0x0B));

			ElementPtr->state_flags |= CHANGING;
		}
	}

	if ((status_flags & SPECIAL)
			&& StarShipPtr->special_counter == 0
			&& DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST))
	{
		SetPrimColor (lpPrim, BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x1F), 0x0F));
		SetPrimType (lpPrim, STAMPFILL_PRIM);

		ProcessSound (SetAbsSoundIndex (
						/* CLOAKING_ON */
				StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1),
				ElementPtr);
		StarShipPtr->special_counter =
				StarShipPtr->RaceDescPtr->characteristics.special_wait;

		ElementPtr->state_flags |= CHANGING;
	}
}

static void
ilwrath_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern,
		COUNT ConcernCounter)
{
	EVALUATE_DESC *lpEvalDesc;
	STARSHIP *StarShipPtr;

	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
	 lpEvalDesc->MoveState = PURSUE;
	if (lpEvalDesc->ObjectPtr && lpEvalDesc->which_turn <= 10)
				/* don't want to dodge when you could be flaming */
		ObjectsOfConcern[ENEMY_WEAPON_INDEX].ObjectPtr = 0;

	ship_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter);

	GetElementStarShip (ShipPtr, &StarShipPtr);
	if (lpEvalDesc->ObjectPtr
			&& (lpEvalDesc->which_turn <= 6
			|| (lpEvalDesc->which_turn <= 10
			&& ObjectsOfConcern[ENEMY_WEAPON_INDEX].which_turn <= 10)))
	{
		StarShipPtr->ship_input_state &= ~SPECIAL;
		if (OBJECT_CLOAKED (ShipPtr))
		{
			StarShipPtr->ship_input_state &= ~LEFT | RIGHT;
			StarShipPtr->ship_input_state |= THRUST;
		}
		StarShipPtr->ship_input_state |= WEAPON;
	}
	else if (StarShipPtr->special_counter == 0
			&& (LOBYTE (GLOBAL (CurrentActivity)) != IN_ENCOUNTER
			|| !GET_GAME_STATE (PROBE_ILWRATH_ENCOUNTER)))
	{
		StarShipPtr->ship_input_state &= ~SPECIAL;
		if (!OBJECT_CLOAKED (ShipPtr)
				&& !(StarShipPtr->ship_input_state & WEAPON))
			StarShipPtr->ship_input_state |= SPECIAL;
	}
}

RACE_DESC*
init_ilwrath (void)
{
	RACE_DESC *RaceDescPtr;

	ilwrath_desc.preprocess_func = ilwrath_preprocess;
    ilwrath_desc.postprocess_func = ilwrath_postprocess;
	ilwrath_desc.init_weapon_func = initialize_flame;
	ilwrath_desc.cyborg_control.intelligence_func = ilwrath_intelligence;

	RaceDescPtr = &ilwrath_desc;

	return (RaceDescPtr);
}

