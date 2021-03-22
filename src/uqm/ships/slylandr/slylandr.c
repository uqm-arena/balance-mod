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
#include "slylandr.h"
#include "resinst.h"
#include "uqm/globdata.h"
#include "libs/mathlib.h"

// Core Characteristics
#define MAX_CREW 12
#define MAX_ENERGY 20
#define ENERGY_REGENERATION 0
#define ENERGY_WAIT 10
#define MAX_THRUST 60
#define THRUST_INCREMENT MAX_THRUST
#define THRUST_WAIT 0
#define TURN_WAIT 0
#define SHIP_MASS 1

// Lightning Weapon
#define WEAPON_ENERGY_COST 2
#define WEAPON_WAIT 17
#define WEAPON_WAIT_ALT 20 // While lightning range is capped, attack time is extended to compensate for penalty
#define SLYLANDRO_OFFSET 9
#define LASER_RANGE 32
#define LASER_RANGE_SHORT 12 // While lightning range is capped, distal lightning segments shrink to this size
#define LASER_LIMIT DISPLAY_TO_WORLD (167)

// Harvester
#define SPECIAL_ENERGY_COST 0
#define SPECIAL_WAIT 20
#define HARVEST_RANGE (208 * 3 / 8)

static RACE_DESC slylandro_desc =
{
	{ /* SHIP_INFO */
		SEEKING_WEAPON | CREW_IMMUNE,
		18, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		SLYLANDRO_RACE_STRINGS,
		SLYLANDRO_ICON_MASK_PMAP_ANIM,
		SLYLANDRO_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		INFINITE_RADIUS, /* Initial sphere of influence radius */
		{ /* Known location (center of SoI) */
			333, 9812,
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
			SLYLANDRO_BIG_MASK_PMAP_ANIM,
			SLYLANDRO_MED_MASK_PMAP_ANIM,
			SLYLANDRO_SML_MASK_PMAP_ANIM,
		},
		{
			NULL_RESOURCE,
			NULL_RESOURCE,
			NULL_RESOURCE,
		},
		{
			NULL_RESOURCE,
			NULL_RESOURCE,
			NULL_RESOURCE,
		},
		{
			SLYLANDRO_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		SLYLANDRO_VICTORY_SONG,
		SLYLANDRO_SHIP_SOUNDS,
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		NULL, NULL
	},
	{
		0,
		CLOSE_RANGE_WEAPON << 1,
		NULL,
	},
	(UNINIT_FUNC *) NULL,
	(PREPROCESS_FUNC *) NULL,
	(POSTPROCESS_FUNC *) NULL,
	(INIT_WEAPON_FUNC *) NULL,
	0,
	0, /* CodeRef */
};

static COUNT initialize_lightning (ELEMENT *ElementPtr,	HELEMENT LaserArray[]);

static void
lightning_postprocess (ELEMENT *ElementPtr)
{
	SIZE delta_x, delta_y;
	STARSHIP *StarShipPtr;
	ELEMENT *ShipPtr;
	HELEMENT Lightning;
	
	GetElementStarShip (ElementPtr, &StarShipPtr);

	LockElement (StarShipPtr->hShip, &ShipPtr);
	
	delta_x = ShipPtr->next.location.x - ElementPtr->next.location.x;
	delta_y = ShipPtr->next.location.y - ElementPtr->next.location.y;

	if (delta_x < 0)
		delta_x = -delta_x;
	if (delta_y < 0)
		delta_y = -delta_y;

	if (StarShipPtr->static_counter > 0
		&& (delta_x + LASER_RANGE >= LASER_LIMIT
			|| delta_y + LASER_RANGE >= LASER_LIMIT
			|| (long)(delta_x + LASER_RANGE) * (delta_x + LASER_RANGE)
			 + (long)(delta_y + LASER_RANGE) * (delta_y + LASER_RANGE)
			>= (long)LASER_LIMIT * LASER_LIMIT))
		ElementPtr->cycle = 2;

	if (ElementPtr->turn_wait
		&& !(ElementPtr->state_flags & COLLISION)
		&& (StarShipPtr->static_counter == 0
			|| (delta_x <= LASER_LIMIT
				&& delta_y <= LASER_LIMIT
				&& (long)delta_x * delta_x + (long)delta_y * delta_y
				<= (long)LASER_LIMIT * LASER_LIMIT)))
	{
		initialize_lightning (ElementPtr, &Lightning);
		if (Lightning)
			PutElement (Lightning);
	}

	UnlockElement (StarShipPtr->hShip);
}

static void
lightning_collision (ELEMENT *ElementPtr0, POINT *pPt0,
		ELEMENT *ElementPtr1, POINT *pPt1)
{
	COUNT weapon_duration;
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr0, &StarShipPtr);

	if (StarShipPtr->static_counter > 0)
		weapon_duration = WEAPON_WAIT_ALT;
	else
		weapon_duration = WEAPON_WAIT;

	if (StarShipPtr->weapon_counter > weapon_duration >> 1)
		StarShipPtr->weapon_counter = weapon_duration - StarShipPtr->weapon_counter;
	StarShipPtr->weapon_counter -= ElementPtr0->turn_wait;
	ElementPtr0->turn_wait = 0;

	weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
}

static COUNT
initialize_lightning (ELEMENT *ElementPtr, HELEMENT LaserArray[])
{
	LASER_BLOCK LaserBlock;

	LaserBlock.cx = ElementPtr->next.location.x;
	LaserBlock.cy = ElementPtr->next.location.y;
	LaserBlock.ex = 0;
	LaserBlock.ey = 0;

	LaserBlock.sender = ElementPtr->playerNr;
	LaserBlock.flags = IGNORE_SIMILAR;
	LaserBlock.face = 0;
	LaserBlock.pixoffs = 0;
	LaserArray[0] = initialize_laser (&LaserBlock);

	if (LaserArray[0])
	{
		SIZE delta;
		COUNT angle, facing, weapon_duration;
		DWORD rand_val;
		ELEMENT *LaserPtr;
		STARSHIP *StarShipPtr;

		GetElementStarShip (ElementPtr, &StarShipPtr);

		if (StarShipPtr->static_counter > 0)
			weapon_duration = WEAPON_WAIT_ALT;
		else
			weapon_duration = WEAPON_WAIT;

		LockElement (LaserArray[0], &LaserPtr);
		LaserPtr->postprocess_func = lightning_postprocess;
		LaserPtr->collision_func = lightning_collision;

		rand_val = TFB_Random ();

		if (!(ElementPtr->state_flags & PLAYER_SHIP))
		{
			angle = GetVelocityTravelAngle (&ElementPtr->velocity);
			facing = NORMALIZE_FACING (ANGLE_TO_FACING (angle));
			delta = TrackAnyShip (ElementPtr, &facing);

			LaserPtr->turn_wait = ElementPtr->turn_wait - 1;

			SetPrimColor (&(GLOBAL (DisplayArray))[LaserPtr->PrimIndex],
					GetPrimColor (&(GLOBAL (DisplayArray))[ElementPtr->PrimIndex]));
		}
		else
		{
			facing = StarShipPtr->ShipFacing;
			ElementPtr->hTarget = 0;
			delta = TrackShip (ElementPtr, &facing);
			ElementPtr->hTarget = 0;
			angle = FACING_TO_ANGLE (facing);

			if ((LaserPtr->turn_wait = StarShipPtr->weapon_counter) == 0)
				LaserPtr->turn_wait = weapon_duration;

			if (LaserPtr->turn_wait > weapon_duration >> 1)
				LaserPtr->turn_wait = weapon_duration - LaserPtr->turn_wait;

			switch (HIBYTE (LOWORD (rand_val)) & 3)
			{
				case 0:
					SetPrimColor (
							&(GLOBAL (DisplayArray))[LaserPtr->PrimIndex],
							BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x1F), 0x0F)
							);
					break;
				case 1:
					SetPrimColor (
							&(GLOBAL (DisplayArray))[LaserPtr->PrimIndex],
							BUILD_COLOR (MAKE_RGB15 (0x16, 0x17, 0x1F), 0x42)
							);
					break;
				case 2:
					SetPrimColor (
							&(GLOBAL (DisplayArray))[LaserPtr->PrimIndex],
							BUILD_COLOR (MAKE_RGB15 (0x06, 0x07, 0x1F), 0x4A)
							);
					break;
				case 3:
					SetPrimColor (
							&(GLOBAL (DisplayArray))[LaserPtr->PrimIndex],
							BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x18), 0x50)
							);
					break;
			}
		}

		if (delta == -1 || delta == ANGLE_TO_FACING (HALF_CIRCLE))
			angle += LOWORD (rand_val);
		else if (delta == 0)
			angle += LOWORD (rand_val) & 1 ? -1 : 1;
		else if (delta < ANGLE_TO_FACING (HALF_CIRCLE))
			angle += LOWORD (rand_val) & (QUADRANT - 1);
		else
			angle -= LOWORD (rand_val) & (QUADRANT - 1);

		// When lightning is almost at max range, shorten the next line
		if (ElementPtr->cycle == 2)
			delta = WORLD_TO_VELOCITY (
				DISPLAY_TO_WORLD ((HIWORD (rand_val) & (LASER_RANGE_SHORT - 1)) + 2));
		else
			delta = WORLD_TO_VELOCITY (
				DISPLAY_TO_WORLD ((HIWORD (rand_val) & (LASER_RANGE - 1)) + 4));

		SetVelocityComponents (&LaserPtr->velocity, COSINE (angle, delta), SINE (angle, delta));

		SetElementStarShip (LaserPtr, StarShipPtr);
		UnlockElement (LaserArray[0]);
	}

	return (1);
}

static BOOLEAN
harvest_space_junk (ELEMENT *ElementPtr)
{
	BOOLEAN retval;
	HELEMENT hElement, hNextElement;

	retval = FALSE;
	for (hElement = GetHeadElement ();
			hElement; hElement = hNextElement)
	{
		ELEMENT *ObjPtr;

		LockElement (hElement, &ObjPtr);
		hNextElement = GetSuccElement (ObjPtr);

		if (!(ObjPtr->state_flags & (APPEARING | PLAYER_SHIP | FINITE_LIFE))
				&& ObjPtr->playerNr == NEUTRAL_PLAYER_NUM
				&& !GRAVITY_MASS (ObjPtr->mass_points)
				&& CollisionPossible (ObjPtr, ElementPtr))
		{
			SIZE dx, dy;

			if ((dx = ObjPtr->next.location.x
					- ElementPtr->next.location.x) < 0)
				dx = -dx;
			if ((dy = ObjPtr->next.location.y
					- ElementPtr->next.location.y) < 0)
				dy = -dy;
			dx = WORLD_TO_DISPLAY (dx);
			dy = WORLD_TO_DISPLAY (dy);
			if (dx <= HARVEST_RANGE && dy <= HARVEST_RANGE
					&& dx * dx + dy * dy <= HARVEST_RANGE * HARVEST_RANGE)
			{
				ObjPtr->life_span = 0;
				ObjPtr->state_flags |= NONSOLID;

				if (!retval)
				{
					STARSHIP *StarShipPtr;

					retval = TRUE;

					GetElementStarShip (ElementPtr, &StarShipPtr);
					ProcessSound (SetAbsSoundIndex (
							StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);
					DeltaEnergy (ElementPtr, MAX_ENERGY);
				}
			}
		}

		UnlockElement (hElement);
	}

	return (retval);
}

static void
slylandro_postprocess (ELEMENT *ElementPtr)
{
	COUNT weapon_duration;
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);

	if (StarShipPtr->static_counter > 0)
		weapon_duration = WEAPON_WAIT_ALT;
	else
		weapon_duration = WEAPON_WAIT;

	if (StarShipPtr->weapon_counter
			&& StarShipPtr->weapon_counter < weapon_duration)
	{
		HELEMENT Lightning;

		initialize_lightning (ElementPtr, &Lightning);
		if (Lightning)
			PutElement (Lightning);
	}

	if (StarShipPtr->special_counter == 0
			&& (StarShipPtr->cur_status_flags & SPECIAL)
			&& harvest_space_junk (ElementPtr))
	{
		StarShipPtr->special_counter =
				StarShipPtr->RaceDescPtr->characteristics.special_wait;
	}
}

static void
slylandro_preprocess (ELEMENT *ElementPtr)
{
	if (!(ElementPtr->state_flags & (APPEARING | NONSOLID)))
	{
		COUNT facing = 0;
		STARSHIP *StarShipPtr, *EnemyStarShipPtr;

		GetElementStarShip (ElementPtr, &StarShipPtr);

		// Identify enemy ship - if it's a certain type, apply weapon range cap
		if (TrackAnyShip (ElementPtr, &facing) >= 0)
		{
			ELEMENT *EnemyPtr;

			LockElement (ElementPtr->hTarget, &EnemyPtr);
			GetElementStarShip (EnemyPtr, &EnemyStarShipPtr);

			if (EnemyStarShipPtr
				&& (EnemyStarShipPtr->SpeciesID == CHMMR_ID
					|| EnemyStarShipPtr->SpeciesID == MMRNMHRM_ID
					|| EnemyStarShipPtr->SpeciesID == VUX_ID
					|| EnemyStarShipPtr->SpeciesID == YEHAT_ID))
				StarShipPtr->static_counter = 1;
			else
				StarShipPtr->static_counter = 0;

			UnlockElement (ElementPtr->hTarget);
		}

		// Directional reverse ability
		if ((StarShipPtr->cur_status_flags & THRUST)
				&& !(StarShipPtr->old_status_flags & THRUST))
			StarShipPtr->ShipFacing += ANGLE_TO_FACING (HALF_CIRCLE);

		// Turn the ship's trajectory
		if (ElementPtr->turn_wait == 0)
		{
			ElementPtr->turn_wait +=
					StarShipPtr->RaceDescPtr->characteristics.turn_wait + 1;
			if (StarShipPtr->cur_status_flags & LEFT)
				--StarShipPtr->ShipFacing;
			else if (StarShipPtr->cur_status_flags & RIGHT)
				++StarShipPtr->ShipFacing;
		}

		StarShipPtr->ShipFacing = NORMALIZE_FACING (StarShipPtr->ShipFacing);

		// Always propel forward
		if (ElementPtr->thrust_wait == 0)
		{
			ElementPtr->thrust_wait +=
					StarShipPtr->RaceDescPtr->characteristics.thrust_wait + 1;

			SetVelocityVector (&ElementPtr->velocity,
					StarShipPtr->RaceDescPtr->characteristics.max_thrust,
					StarShipPtr->ShipFacing);
			StarShipPtr->cur_status_flags |= SHIP_AT_MAX_SPEED;
			StarShipPtr->cur_status_flags &= ~SHIP_IN_GRAVITY_WELL;
		}

		// Rotate ship body regardless of trajectory
		ElementPtr->next.image.frame = IncFrameIndex (ElementPtr->next.image.frame);
		ElementPtr->state_flags |= CHANGING;
	}
}

static void
slylandro_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern,
		COUNT ConcernCounter)
{
	EVALUATE_DESC *lpEvalDesc;
	STARSHIP *StarShipPtr;

	if (LOBYTE (GLOBAL (CurrentActivity)) == IN_ENCOUNTER)
			/* no dodging in role playing game */
		ObjectsOfConcern[ENEMY_WEAPON_INDEX].ObjectPtr = 0;

	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];

	GetElementStarShip (ShipPtr, &StarShipPtr);
	if (StarShipPtr->special_counter == 0
			&& StarShipPtr->RaceDescPtr->ship_info.energy_level == 0
			&& ObjectsOfConcern[GRAVITY_MASS_INDEX].ObjectPtr == 0)
		ConcernCounter = FIRST_EMPTY_INDEX + 1;
	if (lpEvalDesc->ObjectPtr && lpEvalDesc->MoveState == PURSUE
			&& lpEvalDesc->which_turn <= 6)
		lpEvalDesc->MoveState = ENTICE;

	++ShipPtr->thrust_wait;
	ship_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter);
	--ShipPtr->thrust_wait;

	if (lpEvalDesc->ObjectPtr && lpEvalDesc->which_turn <= 14)
		StarShipPtr->ship_input_state |= WEAPON;
	else
		StarShipPtr->ship_input_state &= ~WEAPON;

	StarShipPtr->ship_input_state &= ~SPECIAL;
	if (StarShipPtr->RaceDescPtr->ship_info.energy_level <
			StarShipPtr->RaceDescPtr->ship_info.max_energy)
	{
		lpEvalDesc = &ObjectsOfConcern[FIRST_EMPTY_INDEX];
		if (lpEvalDesc->ObjectPtr && lpEvalDesc->which_turn <= 14)
			StarShipPtr->ship_input_state |= SPECIAL;
	}
}

RACE_DESC*
init_slylandro (void)
{
	RACE_DESC *RaceDescPtr;

	slylandro_desc.preprocess_func = slylandro_preprocess;
	slylandro_desc.postprocess_func = slylandro_postprocess;
	slylandro_desc.init_weapon_func = initialize_lightning;
	slylandro_desc.cyborg_control.intelligence_func = slylandro_intelligence;

	RaceDescPtr = &slylandro_desc;

	return (RaceDescPtr);
}
