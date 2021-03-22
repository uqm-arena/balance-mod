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
#include "supox.h"
#include "resinst.h"
#include "uqm/init.h"
#include "libs/mathlib.h"

// Core Characteristics
#define MAX_CREW 12
#define MAX_ENERGY 12
#define ENERGY_REGENERATION 1
#define ENERGY_WAIT 4
#define MAX_THRUST 40
#define THRUST_INCREMENT 8
#define THRUST_WAIT 0
#define TURN_WAIT 1
#define SHIP_MASS 4

// Gob Launcher
#define WEAPON_ENERGY_COST 1
#define WEAPON_WAIT 2
#define SUPOX_OFFSET 23
#define MISSILE_OFFSET 2
#define MISSILE_SPEED 120
#define MISSILE_LIFE 10
#define MISSILE_HITS 1
#define MISSILE_DAMAGE 1

// Directional Booster
#define SPECIAL_WAIT 8
#define SPECIAL_ENERGY_COST 6
#define DODGE_INCREMENT 28
#define DODGE_MAX_THRUST 80
#define DODGE_DURATION 12

static RACE_DESC supox_desc =
{
	{ /* SHIP_INFO */
		FIRES_FORE,
		14, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		SUPOX_RACE_STRINGS,
		SUPOX_ICON_MASK_PMAP_ANIM,
		SUPOX_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		333 / SPHERE_RADIUS_INCREMENT * 2, /* Initial SoI radius */
		{ /* Known location (center of SoI) */
			7468, 9246,
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
			SUPOX_BIG_MASK_PMAP_ANIM,
			SUPOX_MED_MASK_PMAP_ANIM,
			SUPOX_SML_MASK_PMAP_ANIM,
		},
		{
			GOB_BIG_MASK_PMAP_ANIM,
			GOB_MED_MASK_PMAP_ANIM,
			GOB_SML_MASK_PMAP_ANIM,
		},
		{
			NULL_RESOURCE,
			NULL_RESOURCE,
			NULL_RESOURCE,
		},
		{
			SUPOX_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		SUPOX_VICTORY_SONG,
		SUPOX_SHIP_SOUNDS,
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

static COUNT
initialize_gob (ELEMENT *ShipPtr, HELEMENT GobArray[])
{
	STARSHIP *StarShipPtr;
	MISSILE_BLOCK MissileBlock;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	MissileBlock.cx = ShipPtr->next.location.x;
	MissileBlock.cy = ShipPtr->next.location.y;
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.face = MissileBlock.index = StarShipPtr->ShipFacing;
	MissileBlock.sender = ShipPtr->playerNr;
	MissileBlock.flags = IGNORE_SIMILAR;
	MissileBlock.pixoffs = SUPOX_OFFSET;
	MissileBlock.speed = MISSILE_SPEED;
	MissileBlock.hit_points = MISSILE_HITS;
	MissileBlock.damage = MISSILE_DAMAGE;
	MissileBlock.life = MISSILE_LIFE;
	MissileBlock.preprocess_func = NULL;
	MissileBlock.blast_offs = MISSILE_OFFSET;
	GobArray[0] = initialize_missile (&MissileBlock);
	
	if (GobArray[0])
	{
		SIZE dx, dy;
		ELEMENT *MissilePtr;

		LockElement (GobArray[0], &MissilePtr);

		GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
		dx = dx >> 1;
		dy = dy >> 1;

		// Add some of the Blade's velocity to its projectiles
		DeltaVelocityComponents (&MissilePtr->velocity, dx, dy);
		MissilePtr->current.location.x -= VELOCITY_TO_WORLD (dx);
		MissilePtr->current.location.y -= VELOCITY_TO_WORLD (dy);

		UnlockElement (GobArray[0]);
	}
	
	return (1);
}

// Spawn exhaust particles at the front of the ship while active
static void
reverse_ion_trail (ELEMENT *ElementPtr)
{
#define START_ION_COLOR BUILD_COLOR (MAKE_RGB15 (0x1F, 0x17, 0x00), 0x7a)
	
	if (ElementPtr->state_flags & PLAYER_SHIP)
	{
		HELEMENT hIonElement;

		hIonElement = AllocElement ();
		if (hIonElement)
		{
			COUNT angle;
			RECT r;
			ELEMENT *IonElementPtr;
			STARSHIP *StarShipPtr;

			GetElementStarShip (ElementPtr, &StarShipPtr);
			angle = FACING_TO_ANGLE (StarShipPtr->ShipFacing);
			GetFrameRect (StarShipPtr->RaceDescPtr->ship_data.ship[0], &r);
			r.extent.height = DISPLAY_TO_WORLD (r.extent.height + r.corner.y);

			InsertElement (hIonElement, GetHeadElement ());
			LockElement (hIonElement, &IonElementPtr);
			IonElementPtr->state_flags = APPEARING | FINITE_LIFE | NONSOLID;
			IonElementPtr->life_span = IonElementPtr->thrust_wait = 1;
			SetPrimType (&DisplayArray[IonElementPtr->PrimIndex], POINT_PRIM);
			SetPrimColor (&DisplayArray[IonElementPtr->PrimIndex], START_ION_COLOR);
			IonElementPtr->current.image.frame = DecFrameIndex (stars_in_space);
			IonElementPtr->current.image.farray = &stars_in_space;
			IonElementPtr->current.location = ElementPtr->current.location;
			IonElementPtr->current.location.x += (COORD)COSINE (angle, r.extent.height);
			IonElementPtr->current.location.y += (COORD)SINE (angle, r.extent.height);
			IonElementPtr->death_func = reverse_ion_trail;
			IonElementPtr->turn_wait = 0;

			SetElementStarShip (IonElementPtr, StarShipPtr);

			{
				/* normally done during preprocess, but because
				 * object is being inserted at head rather than
				 * appended after tail it may never get preprocessed.
				 */
				IonElementPtr->next = IonElementPtr->current;
				--IonElementPtr->life_span;
				IonElementPtr->state_flags |= PRE_PROCESS;
			}

			UnlockElement (hIonElement);
		}
	}
	else
	{
		static const Color colorTab[] =
		{
			BUILD_COLOR (MAKE_RGB15_INIT (0x1F, 0x14, 0x00), 0x7a),
			BUILD_COLOR (MAKE_RGB15_INIT (0x1F, 0x11, 0x00), 0x7b),
			BUILD_COLOR (MAKE_RGB15_INIT (0x1F, 0x0E, 0x00), 0x7c),
			BUILD_COLOR (MAKE_RGB15_INIT (0x1F, 0x0B, 0x00), 0x7d),
			BUILD_COLOR (MAKE_RGB15_INIT (0x1F, 0x08, 0x00), 0x7e),
			BUILD_COLOR (MAKE_RGB15_INIT (0x1F, 0x05, 0x00), 0x7f),
			BUILD_COLOR (MAKE_RGB15_INIT (0x1F, 0x02, 0x00), 0x2a),
			BUILD_COLOR (MAKE_RGB15_INIT (0x1B, 0x00, 0x00), 0x2b),
			BUILD_COLOR (MAKE_RGB15_INIT (0x17, 0x00, 0x00), 0x2c),
			BUILD_COLOR (MAKE_RGB15_INIT (0x13, 0x00, 0x00), 0x2d),
			BUILD_COLOR (MAKE_RGB15_INIT (0x0F, 0x00, 0x00), 0x2e),
			BUILD_COLOR (MAKE_RGB15_INIT (0x0B, 0x00, 0x00), 0x2f),
		};

		const size_t colorTabCount = sizeof colorTab / sizeof colorTab[0];
		
		assert (!(ElementPtr->state_flags & PLAYER_SHIP));
		
		if (ElementPtr->turn_wait != colorTabCount)
		{
			ElementPtr->life_span = ElementPtr->thrust_wait;
					// Reset the life span.
		
			++ElementPtr->turn_wait;
			
			SetPrimColor (&DisplayArray[ElementPtr->PrimIndex],
					colorTab[ElementPtr->colorCycleIndex]);
			
			ElementPtr->state_flags &= ~DISAPPEARING;
			ElementPtr->state_flags |= CHANGING;
		} // else, the element disappears.
		
		ElementPtr->colorCycleIndex++;
	}
}

// After-image effect for dodge ability
static void
dodge_trail (ELEMENT *ElementPtr)
{
#define START_DODGE_COLOR BUILD_COLOR (MAKE_RGB15 (0x1F, 0x11, 0x00), 0x7a)

	if (ElementPtr->state_flags & PLAYER_SHIP)
	{
		HELEMENT hTrailElement;

		hTrailElement = AllocElement ();
		if (hTrailElement)
		{
			HELEMENT hHeadElement;
			ELEMENT *TrailElementPtr, *HeadElementPtr;
			STARSHIP *StarShipPtr;

			GetElementStarShip (ElementPtr, &StarShipPtr);

			hHeadElement = GetHeadElement();
			LockElement (hHeadElement, &HeadElementPtr);
			if (HeadElementPtr == ElementPtr)
				InsertElement (hTrailElement, GetHeadElement ());
			else
				InsertElement (hTrailElement, GetPredElement (ElementPtr));
			UnlockElement(hHeadElement);

			LockElement (hTrailElement, &TrailElementPtr);
			TrailElementPtr->state_flags = APPEARING | FINITE_LIFE | NONSOLID;
			TrailElementPtr->life_span = TrailElementPtr->thrust_wait = 1;
			SetPrimType (&DisplayArray[TrailElementPtr->PrimIndex], STAMPFILL_PRIM);
			SetPrimColor (&DisplayArray[TrailElementPtr->PrimIndex], START_DODGE_COLOR);
			TrailElementPtr->current.image.frame = ElementPtr->current.image.frame;
			TrailElementPtr->current.image.farray = ElementPtr->current.image.farray;
			TrailElementPtr->current.location = ElementPtr->current.location;
			TrailElementPtr->death_func = dodge_trail;
			TrailElementPtr->turn_wait = 0;

			SetElementStarShip (TrailElementPtr, StarShipPtr);

			{
				/* normally done during preprocess, but because
				 * object is being inserted at head rather than
				 * appended after tail it may never get preprocessed.
				 */
				TrailElementPtr->next = TrailElementPtr->current;
				--TrailElementPtr->life_span;
				TrailElementPtr->state_flags |= PRE_PROCESS;
			}

			UnlockElement (hTrailElement);
		}
	}
	else
	{
		static const Color colorTab[] =
		{
			BUILD_COLOR (MAKE_RGB15_INIT (0x1D, 0x0A, 0x00), 0x7c),
			BUILD_COLOR (MAKE_RGB15_INIT (0x1A, 0x06, 0x00), 0x7c),
			BUILD_COLOR (MAKE_RGB15_INIT (0x17, 0x03, 0x00), 0x2c),
			BUILD_COLOR (MAKE_RGB15_INIT (0x11, 0x02, 0x00), 0x2c),
			BUILD_COLOR (MAKE_RGB15_INIT (0x09, 0x01, 0x00), 0x2f),
		};

		const size_t colorTabCount = sizeof colorTab / sizeof colorTab[0];
		
		assert (!(ElementPtr->state_flags & PLAYER_SHIP));
		
		if (ElementPtr->turn_wait != colorTabCount)
		{
			ElementPtr->life_span = ElementPtr->thrust_wait;
				// Reset the life span
		
			++ElementPtr->turn_wait;
			
			SetPrimColor (&DisplayArray[ElementPtr->PrimIndex],
					colorTab[ElementPtr->colorCycleIndex]);
			
			ElementPtr->state_flags &= ~DISAPPEARING;
			ElementPtr->state_flags |= CHANGING;
		} // else, the element disappears.
		
		ElementPtr->colorCycleIndex++;
	}
}

static void
supox_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;
	COUNT facing, reverse_facing, add_facing, max_thrust, thrust_increment;
	STATUS_FLAGS thrust_status;
	HELEMENT hTrailElement;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	
	facing = StarShipPtr->ShipFacing;
	reverse_facing = 0;
	
	// 'Down' key initiates reverse thrust
	if (StarShipPtr->cur_status_flags & DOWN
			&& !(StarShipPtr->cur_status_flags & SPECIAL)
			&& StarShipPtr->static_counter == 0)
		reverse_facing = ANGLE_TO_FACING (HALF_CIRCLE);

	// End dodge
	if (StarShipPtr->static_counter > 0
			&& StarShipPtr->special_counter == 0)
	{
		StarShipPtr->static_counter = 0;
		
		facing = NORMALIZE_FACING (ANGLE_TO_FACING (GetVelocityTravelAngle (&ElementPtr->velocity)));
		
		// Reduce speed back down to normal limit
		if (!(StarShipPtr->cur_status_flags & SHIP_IN_GRAVITY_WELL))
			SetVelocityVector (&ElementPtr->velocity, MAX_THRUST, NORMALIZE_FACING (facing));
		else
			SetVelocityVector (&ElementPtr->velocity, 72, NORMALIZE_FACING (facing));

		StarShipPtr->special_counter = SPECIAL_WAIT;
	}

	// Start dodge
	if (StarShipPtr->cur_status_flags & SPECIAL)
	{
		++ElementPtr->turn_wait; // Stall turning

		if (StarShipPtr->cur_status_flags & (THRUST | DOWN | LEFT | RIGHT)
				&& StarShipPtr->RaceDescPtr->ship_info.energy_level >= SPECIAL_ENERGY_COST			
				&& StarShipPtr->special_counter == 0)
		{
			// Set direction of dodge using this very inelegant solution
			if (StarShipPtr->cur_status_flags & THRUST)
				if (StarShipPtr->cur_status_flags & LEFT)
					StarShipPtr->static_counter = 5;
				else if (StarShipPtr->cur_status_flags & RIGHT)
					StarShipPtr->static_counter = 6;
				else
					StarShipPtr->static_counter = 1;
			else if (StarShipPtr->cur_status_flags & DOWN)
				if (StarShipPtr->cur_status_flags & LEFT)
					StarShipPtr->static_counter = 7;
				else if (StarShipPtr->cur_status_flags & RIGHT)
					StarShipPtr->static_counter = 8;
				else
					StarShipPtr->static_counter = 2;
			else if (StarShipPtr->cur_status_flags & LEFT)
			{
				StarShipPtr->static_counter = 3;

				if (ElementPtr->turn_wait == 0)
					++ElementPtr->turn_wait;
			}
			else if (StarShipPtr->cur_status_flags & RIGHT)
			{
				StarShipPtr->static_counter = 4;

				if (ElementPtr->turn_wait == 0)
					++ElementPtr->turn_wait;
			}

			ElementPtr->thrust_wait += DODGE_DURATION + 1;
			StarShipPtr->special_counter = DODGE_DURATION + 1;

			DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST);
		
			ProcessSound (SetAbsSoundIndex (
				/* Boost */
				StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);
		}
	}

	// Dodge is active
	if (StarShipPtr->static_counter > 0)
	{
		if (StarShipPtr->static_counter == 1) // Forward
			add_facing = 0;
		else if (StarShipPtr->static_counter == 2) // Backward
			add_facing = ANGLE_TO_FACING (HALF_CIRCLE);
		else if (StarShipPtr->static_counter == 3)  // Left
			add_facing = -ANGLE_TO_FACING (QUADRANT);
		else if (StarShipPtr->static_counter == 4) // Right
			add_facing = ANGLE_TO_FACING (QUADRANT);
		else if (StarShipPtr->static_counter == 5) // Up-Left
			add_facing = -ANGLE_TO_FACING (OCTANT);
		else if (StarShipPtr->static_counter == 6) // Up-Right
			add_facing = ANGLE_TO_FACING (OCTANT);
		else if (StarShipPtr->static_counter == 7) // Down-Left
			add_facing = ANGLE_TO_FACING (HALF_CIRCLE + OCTANT);
		else if (StarShipPtr->static_counter == 8) // Down-Right
			add_facing = ANGLE_TO_FACING (HALF_CIRCLE - OCTANT);
			
		StarShipPtr->ShipFacing = NORMALIZE_FACING (facing + add_facing);

		StarShipPtr->cur_status_flags &= ~(SHIP_AT_MAX_SPEED | SHIP_BEYOND_MAX_SPEED);

		thrust_increment = StarShipPtr->RaceDescPtr->characteristics.thrust_increment;
		max_thrust = StarShipPtr->RaceDescPtr->characteristics.max_thrust;
		StarShipPtr->RaceDescPtr->characteristics.thrust_increment = DODGE_INCREMENT;
		StarShipPtr->RaceDescPtr->characteristics.max_thrust = DODGE_MAX_THRUST;
		
		thrust_status = inertial_thrust (ElementPtr);
		StarShipPtr->cur_status_flags &=
				~(SHIP_AT_MAX_SPEED
				| SHIP_BEYOND_MAX_SPEED
				| SHIP_IN_GRAVITY_WELL);
		StarShipPtr->cur_status_flags |= thrust_status;
		
		StarShipPtr->ShipFacing = facing;
		StarShipPtr->RaceDescPtr->characteristics.thrust_increment = thrust_increment;
		StarShipPtr->RaceDescPtr->characteristics.max_thrust = max_thrust;

		++StarShipPtr->energy_counter; // Stall battery regen

		Untarget (ElementPtr); // Shake off most forms of enemy tracking
	}

	// Conventional reverse thrust
	if (reverse_facing
		&& ElementPtr->thrust_wait == 0)
	{
		StarShipPtr->ShipFacing = NORMALIZE_FACING (facing + reverse_facing);
		thrust_status = inertial_thrust (ElementPtr);
		StarShipPtr->cur_status_flags &=
				~(SHIP_AT_MAX_SPEED
				| SHIP_BEYOND_MAX_SPEED
				| SHIP_IN_GRAVITY_WELL);
		StarShipPtr->cur_status_flags |= thrust_status;
		StarShipPtr->ShipFacing = facing;

		reverse_ion_trail(ElementPtr);
	}

	if (StarShipPtr->static_counter > 0)
		dodge_trail(ElementPtr);
}

static void
supox_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern, COUNT ConcernCounter)
{
	STARSHIP *StarShipPtr;
	EVALUATE_DESC *lpEvalDesc;

	GetElementStarShip (ShipPtr, &StarShipPtr);

	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
	if (StarShipPtr->static_counter || lpEvalDesc->ObjectPtr == 0)
		StarShipPtr->ship_input_state &= ~DOWN;
	else
	{
		BOOLEAN LinedUp;
		COUNT direction_angle;
		SIZE delta_x, delta_y;

		delta_x = lpEvalDesc->ObjectPtr->next.location.x
				- ShipPtr->next.location.x;
		delta_y = lpEvalDesc->ObjectPtr->next.location.y
				- ShipPtr->next.location.y;
		direction_angle = ARCTAN (delta_x, delta_y);

		LinedUp = (BOOLEAN)(NORMALIZE_ANGLE (NORMALIZE_ANGLE (direction_angle
				- FACING_TO_ANGLE (StarShipPtr->ShipFacing))
				+ QUADRANT) <= HALF_CIRCLE);

		if (!LinedUp
				|| lpEvalDesc->which_turn > 20
				|| NORMALIZE_ANGLE (
				lpEvalDesc->facing
				- (FACING_TO_ANGLE (StarShipPtr->ShipFacing)
				+ HALF_CIRCLE) + OCTANT
				) > QUADRANT)
			StarShipPtr->ship_input_state &= ~DOWN;
		else if (LinedUp && lpEvalDesc->which_turn <= 12)
			StarShipPtr->ship_input_state |= DOWN;

		if (StarShipPtr->ship_input_state & DOWN)
		{
			StarShipPtr->ship_input_state &= ~THRUST;
			lpEvalDesc->MoveState = PURSUE;
		}
	}

	ship_intelligence (ShipPtr,	ObjectsOfConcern, ConcernCounter);

	if (StarShipPtr->ship_input_state & DOWN)
		StarShipPtr->ship_input_state |= WEAPON;

	lpEvalDesc = &ObjectsOfConcern[ENEMY_WEAPON_INDEX];
	if (lpEvalDesc->ObjectPtr && lpEvalDesc->which_turn <= 6
			&& StarShipPtr->special_counter == 0
			&& StarShipPtr->RaceDescPtr->ship_info.energy_level >=
				StarShipPtr->RaceDescPtr->ship_info.max_energy >> 1)
	{
		BOOLEAN IsTrackingWeapon;
		STARSHIP *EnemyStarShipPtr;

		GetElementStarShip (lpEvalDesc->ObjectPtr, &EnemyStarShipPtr);
		if (((EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags
				& SEEKING_WEAPON) &&
				lpEvalDesc->ObjectPtr->next.image.farray ==
				EnemyStarShipPtr->RaceDescPtr->ship_data.weapon) ||
				((EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags
				& SEEKING_SPECIAL) &&
				lpEvalDesc->ObjectPtr->next.image.farray ==
				EnemyStarShipPtr->RaceDescPtr->ship_data.special))
			IsTrackingWeapon = TRUE;
		else
			IsTrackingWeapon = FALSE;

		if (((lpEvalDesc->ObjectPtr->state_flags & PLAYER_SHIP) /* means IMMEDIATE WEAPON */
			|| (IsTrackingWeapon
				&& (lpEvalDesc->which_turn <= 2
					|| (lpEvalDesc->ObjectPtr->state_flags & CREW_OBJECT))) /* FIGHTERS!!! */
			|| PlotIntercept (lpEvalDesc->ObjectPtr, ShipPtr, 3, 0))
			&& (TFB_Random () & 4))

		StarShipPtr->ship_input_state &= ~THRUST;
		StarShipPtr->ship_input_state |= SPECIAL;

		if (!(StarShipPtr->cur_status_flags & (LEFT | RIGHT)))
			StarShipPtr->ship_input_state |= 1 << ((BYTE)TFB_Random () & 1);
		else
			StarShipPtr->ship_input_state |=
					StarShipPtr->cur_status_flags & (LEFT | RIGHT);
	}
	else if (StarShipPtr->ship_input_state & SPECIAL)
		StarShipPtr->ship_input_state &= ~SPECIAL;
}

RACE_DESC*
init_supox (void)
{
	RACE_DESC *RaceDescPtr;

	supox_desc.preprocess_func = supox_preprocess;
	supox_desc.init_weapon_func = initialize_gob;
	supox_desc.cyborg_control.intelligence_func = supox_intelligence;

	RaceDescPtr = &supox_desc;

	return (RaceDescPtr);
}

