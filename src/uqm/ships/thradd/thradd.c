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
#include "thradd.h"
#include "resinst.h"
#include "uqm/globdata.h"
#include "libs/mathlib.h"

// Core Characteristics
#define MAX_CREW 10
#define MAX_ENERGY 16
#define ENERGY_REGENERATION 1
#define ENERGY_WAIT 6
#define MAX_THRUST 28
#define THRUST_INCREMENT 7
#define THRUST_WAIT 0
#define TURN_WAIT 1
#define SHIP_MASS 4

// Ion Blasters
#define WEAPON_ENERGY_COST 2
#define WEAPON_WAIT 3
#define MISSILE_SPEED 72
#define MISSILE_LIFE 10
#define MISSILE_OFFSET 8
#define THRADDASH_OFFSET_MID 17
#define THRADDASH_OFFSET_SIDE 9
#define MISSILE_HITS 2
#define MISSILE_DAMAGE 1

// Afterburner
#define SPECIAL_ENERGY_COST 1
#define SPECIAL_WAIT 0
#define SPECIAL_THRUST_INCREMENT 12
#define SPECIAL_MAX_THRUST 72
#define NAPALM_LIFE 60
#define NAPALM_OFFSET 0
#define NAPALM_HITS 2
#define NAPALM_DAMAGE 2
#define NUM_NAPALM_FADES 6
#define NAPALM_FADE_WAIT 2

static RACE_DESC thraddash_desc =
{
	{ /* SHIP_INFO */
		FIRES_FORE,
		10, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		THRADDASH_RACE_STRINGS,
		THRADDASH_ICON_MASK_PMAP_ANIM,
		THRADDASH_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		833 / SPHERE_RADIUS_INCREMENT * 2, /* Initial SoI radius */
		{ /* Known location (center of SoI) */
			2535, 8358,
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
			THRADDASH_BIG_MASK_PMAP_ANIM,
			THRADDASH_MED_MASK_PMAP_ANIM,
			THRADDASH_SML_MASK_PMAP_ANIM,
		},
		{
			HORN_BIG_MASK_PMAP_ANIM,
			HORN_MED_MASK_PMAP_ANIM,
			HORN_SML_MASK_PMAP_ANIM,
			// REPULSOR_BIG_MASK_PMAP_ANIM,
			// REPULSOR_MED_MASK_PMAP_ANIM,
			// REPULSOR_SML_MASK_PMAP_ANIM,
		},
		{
			NAPALM_BIG_MASK_PMAP_ANIM,
			NAPALM_MED_MASK_PMAP_ANIM,
			NAPALM_SML_MASK_PMAP_ANIM,
		},
		{
			THRADDASH_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		THRADDASH_VICTORY_SONG,
		THRADDASH_SHIP_SOUNDS,
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		NULL, NULL
	},
	{
		0,
		(MISSILE_SPEED * MISSILE_LIFE) >> 1,
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
thraddash_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern, COUNT ConcernCounter)
{

	STARSHIP *StarShipPtr;
	EVALUATE_DESC *lpEvalDesc;
	
	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
	if (lpEvalDesc->ObjectPtr)
	{
#define STATIONARY_SPEED WORLD_TO_VELOCITY (DISPLAY_TO_WORLD (4))
		SIZE dx, dy;

		GetCurrentVelocityComponents (
				&lpEvalDesc->ObjectPtr->velocity, &dx, &dy
				);
		if (lpEvalDesc->which_turn > 8
				|| (long)dx * dx + (long)dy * dy <=
				(long)STATIONARY_SPEED * STATIONARY_SPEED)
			lpEvalDesc->MoveState = PURSUE;
		else
			lpEvalDesc->MoveState = ENTICE;
	}
	ship_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter);

	GetElementStarShip (ShipPtr, &StarShipPtr);
	if (StarShipPtr->special_counter == 0)
	{
		StarShipPtr->ship_input_state &= ~SPECIAL;
		if (ObjectsOfConcern[ENEMY_WEAPON_INDEX].ObjectPtr
				&& ObjectsOfConcern[ENEMY_WEAPON_INDEX].MoveState == ENTICE)
		{
			if ((StarShipPtr->ship_input_state & THRUST)
					|| (ShipPtr->turn_wait == 0
					&& !(StarShipPtr->ship_input_state & (LEFT | RIGHT)))
					|| NORMALIZE_FACING (ANGLE_TO_FACING (
					GetVelocityTravelAngle (
					&ObjectsOfConcern[ENEMY_WEAPON_INDEX].ObjectPtr->velocity
					) + HALF_CIRCLE + OCTANT)
					- StarShipPtr->ShipFacing) > ANGLE_TO_FACING (QUADRANT))
				StarShipPtr->ship_input_state |= SPECIAL;
		}
		else if (lpEvalDesc->ObjectPtr)
		{
			if (lpEvalDesc->MoveState == PURSUE)
			{
				if (StarShipPtr->RaceDescPtr->ship_info.energy_level >= WEAPON_ENERGY_COST
						+ SPECIAL_ENERGY_COST
						&& ShipPtr->turn_wait == 0
						&& !(StarShipPtr->ship_input_state & (LEFT | RIGHT))
						&& (!(StarShipPtr->cur_status_flags & SPECIAL)
						|| !(StarShipPtr->cur_status_flags
						& (SHIP_AT_MAX_SPEED | SHIP_BEYOND_MAX_SPEED))))
					StarShipPtr->ship_input_state |= SPECIAL;
			}
			else if (lpEvalDesc->MoveState == ENTICE)
			{
				COUNT direction_angle;
				SIZE delta_x, delta_y;

				delta_x = lpEvalDesc->ObjectPtr->next.location.x
						- ShipPtr->next.location.x;
				delta_y = lpEvalDesc->ObjectPtr->next.location.y
						- ShipPtr->next.location.y;
				direction_angle = ARCTAN (delta_x, delta_y);

				if ((lpEvalDesc->which_turn > 24
						&& !(StarShipPtr->ship_input_state & (LEFT | RIGHT)))
						|| (lpEvalDesc->which_turn <= 16
						&& NORMALIZE_ANGLE (direction_angle
						- (FACING_TO_ANGLE (StarShipPtr->ShipFacing) + HALF_CIRCLE)
						+ QUADRANT) <= HALF_CIRCLE
						&& (lpEvalDesc->which_turn < 12
						|| NORMALIZE_ANGLE (direction_angle
						- (GetVelocityTravelAngle (
								&lpEvalDesc->ObjectPtr->velocity
								) + HALF_CIRCLE)
						+ (OCTANT + 2)) <= ((OCTANT + 2) << 1))))
					StarShipPtr->ship_input_state |= SPECIAL;
			}
		}

		if ((StarShipPtr->ship_input_state & SPECIAL)
				&& StarShipPtr->RaceDescPtr->ship_info.energy_level >=
				SPECIAL_ENERGY_COST)
			StarShipPtr->ship_input_state &= ~THRUST;
	}
}

static COUNT
initialize_weapon (ELEMENT *ShipPtr, HELEMENT WeaponArray[])
{
	COUNT i;
	STARSHIP *StarShipPtr;
	MISSILE_BLOCK MissileBlock;
	
	GetElementStarShip (ShipPtr, &StarShipPtr);
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.face = MissileBlock.index = StarShipPtr->ShipFacing;
	MissileBlock.sender = ShipPtr->playerNr;
	MissileBlock.flags = IGNORE_SIMILAR;
	MissileBlock.speed = MISSILE_SPEED;
	MissileBlock.hit_points = MISSILE_HITS;
	MissileBlock.damage = MISSILE_DAMAGE;
	MissileBlock.life = MISSILE_LIFE;
	MissileBlock.blast_offs = MISSILE_OFFSET;
	MissileBlock.preprocess_func = 0;

	for (i = 0; i < 2; ++i)
	{
		if (i == 0)
		{
			MissileBlock.pixoffs = THRADDASH_OFFSET_MID;
			MissileBlock.cx = ShipPtr->next.location.x;
			MissileBlock.cy = ShipPtr->next.location.y;
		}
		else if (StarShipPtr->static_counter == 0)
		{
			MissileBlock.pixoffs = THRADDASH_OFFSET_SIDE;
			MissileBlock.cx = ShipPtr->next.location.x
				+ COSINE(FACING_TO_ANGLE (MissileBlock.face + 4), -28);
			MissileBlock.cy = ShipPtr->next.location.y
				+ SINE(FACING_TO_ANGLE (MissileBlock.face + 4), -28);
		}
		else
		{
			MissileBlock.pixoffs = THRADDASH_OFFSET_SIDE;
			MissileBlock.cx = ShipPtr->next.location.x
				+ COSINE(FACING_TO_ANGLE (MissileBlock.face + 4), 28);
			MissileBlock.cy = ShipPtr->next.location.y
				+ SINE(FACING_TO_ANGLE (MissileBlock.face + 4), 28);
		}

		if ((WeaponArray[i] = initialize_missile (&MissileBlock)))
		{
			SIZE dx, dy;
			ELEMENT *WeaponPtr;

			LockElement (WeaponArray[i], &WeaponPtr);

			GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
			dx = dx * 3/4;
			dy = dy * 3/4;

			// Add some of the Torch's velocity to its projectiles
			DeltaVelocityComponents (&WeaponPtr->velocity, dx, dy);
			WeaponPtr->current.location.x -= VELOCITY_TO_WORLD (dx);
			WeaponPtr->current.location.y -= VELOCITY_TO_WORLD (dy);
			// WeaponPtr->collision_func = repulsor_collision;		
			UnlockElement (WeaponArray[i]);
		}
	}

	return (2);
}

static void
flame_napalm_preprocess (ELEMENT *ElementPtr)
{
	ZeroVelocityComponents (&ElementPtr->velocity);

	if (ElementPtr->state_flags & NONSOLID)
	{
		ElementPtr->state_flags &= ~NONSOLID;
		ElementPtr->state_flags |= APPEARING;
		SetPrimType (&(GLOBAL (DisplayArray))[ElementPtr->PrimIndex],
				STAMP_PRIM);

		InitIntersectStartPoint (ElementPtr);
		InitIntersectEndPoint (ElementPtr);
		InitIntersectFrame (ElementPtr);
	}
	else if (ElementPtr->turn_wait > 0)
		--ElementPtr->turn_wait;
	else
	{
		if (ElementPtr->life_span <= NUM_NAPALM_FADES * (NAPALM_FADE_WAIT + 1)
				|| GetFrameIndex (
				ElementPtr->current.image.frame
				) != NUM_NAPALM_FADES)
			ElementPtr->next.image.frame =
					DecFrameIndex (ElementPtr->current.image.frame);
		else if (ElementPtr->life_span > NUM_NAPALM_FADES * (NAPALM_FADE_WAIT + 1))
			ElementPtr->next.image.frame = SetAbsFrameIndex (
					ElementPtr->current.image.frame,
					GetFrameCount (ElementPtr->current.image.frame) - 1
					);

		ElementPtr->turn_wait = NAPALM_FADE_WAIT;
		ElementPtr->state_flags |= CHANGING;
	}
}

static void
thraddash_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;
	
	GetElementStarShip (ElementPtr, &StarShipPtr);
	
	// Switch side guns after every shot
	if (StarShipPtr->weapon_counter == 1)
	{
		if (StarShipPtr->static_counter == 0)
			++StarShipPtr->static_counter;
		else
			--StarShipPtr->static_counter;
	}

	if (!(StarShipPtr->cur_status_flags & SPECIAL))
	{
		if (StarShipPtr->old_status_flags & SPECIAL
				&& StarShipPtr->cur_status_flags & SHIP_AT_MAX_SPEED)
			StarShipPtr->cur_status_flags |= SHIP_BEYOND_MAX_SPEED;
	}
	else if (StarShipPtr->RaceDescPtr->ship_info.energy_level >= SPECIAL_ENERGY_COST)
	{
		COUNT max_thrust, thrust_increment;
		STATUS_FLAGS thrust_status;
		HELEMENT hTrailElement;

		if (!(StarShipPtr->old_status_flags & SPECIAL))
			StarShipPtr->cur_status_flags &=
					~(SHIP_AT_MAX_SPEED | SHIP_BEYOND_MAX_SPEED);

		if (ElementPtr->thrust_wait == 0)
			++ElementPtr->thrust_wait;

		thrust_increment = StarShipPtr->RaceDescPtr->characteristics.thrust_increment;
		max_thrust = StarShipPtr->RaceDescPtr->characteristics.max_thrust;
		StarShipPtr->RaceDescPtr->characteristics.thrust_increment = SPECIAL_THRUST_INCREMENT;
		StarShipPtr->RaceDescPtr->characteristics.max_thrust = SPECIAL_MAX_THRUST;

		thrust_status = inertial_thrust (ElementPtr);
		StarShipPtr->cur_status_flags &=
				~(SHIP_AT_MAX_SPEED
				| SHIP_BEYOND_MAX_SPEED
				| SHIP_IN_GRAVITY_WELL);
		StarShipPtr->cur_status_flags |= thrust_status;

		StarShipPtr->RaceDescPtr->characteristics.thrust_increment = thrust_increment;
		StarShipPtr->RaceDescPtr->characteristics.max_thrust = max_thrust;
		
		// Reduce afterburner energy consumption by 1/3
		if (StarShipPtr->special_counter == 0)
		{
			DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST);

			StarShipPtr->special_counter = 2;
		}
		else if (StarShipPtr->special_counter == 2)
			DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST);
		
		{
			MISSILE_BLOCK MissileBlock;

			MissileBlock.cx = ElementPtr->next.location.x;
			MissileBlock.cy = ElementPtr->next.location.y;
			MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.special;
			MissileBlock.face = 0;
			MissileBlock.index = GetFrameCount (StarShipPtr->RaceDescPtr->ship_data.special[0]) - 1;
			MissileBlock.sender = ElementPtr->playerNr;
			MissileBlock.flags = IGNORE_SIMILAR;
			MissileBlock.pixoffs = 0;
			MissileBlock.speed = 0;
			MissileBlock.hit_points = NAPALM_HITS;
			MissileBlock.damage = NAPALM_DAMAGE;
			MissileBlock.life = NAPALM_LIFE;
			MissileBlock.preprocess_func = flame_napalm_preprocess;
			MissileBlock.blast_offs = NAPALM_OFFSET;

			hTrailElement = initialize_missile (&MissileBlock);
			if (hTrailElement)
			{
				ELEMENT *TrailElementPtr;

				LockElement (hTrailElement, &TrailElementPtr);
				SetElementStarShip (TrailElementPtr, StarShipPtr);
				TrailElementPtr->hTarget = 0;
				TrailElementPtr->turn_wait = NAPALM_FADE_WAIT;

				TrailElementPtr->state_flags |= NONSOLID;
				SetPrimType (
						&(GLOBAL (DisplayArray))[TrailElementPtr->PrimIndex],
						NO_PRIM
						);

						/* normally done during preprocess, but because
						 * object is being inserted at head rather than
						 * appended after tail it may never get preprocessed.
						 */
				TrailElementPtr->next = TrailElementPtr->current;
				TrailElementPtr->state_flags |= PRE_PROCESS;

				UnlockElement (hTrailElement);
				InsertElement (hTrailElement, GetHeadElement ());
				
				ProcessSound (SetAbsSoundIndex (
						/* BURNT_TOAST */
					StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);
			}
		}
	}
	else if (StarShipPtr->RaceDescPtr->ship_info.energy_level < SPECIAL_ENERGY_COST)
		DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST); /* so text will flash */
}

RACE_DESC*
init_thraddash (void)
{
	RACE_DESC *RaceDescPtr;

	thraddash_desc.preprocess_func = thraddash_preprocess;
	thraddash_desc.init_weapon_func = initialize_weapon;
	thraddash_desc.cyborg_control.intelligence_func = thraddash_intelligence;

	RaceDescPtr = &thraddash_desc;

	return (RaceDescPtr);
}
