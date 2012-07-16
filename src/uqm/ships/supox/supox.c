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

// Core characteristics
#define MAX_CREW 12
#define MAX_ENERGY 12
#define ENERGY_REGENERATION 1
#define ENERGY_WAIT 5
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

// Alternate Thrust
#define SPECIAL_ENERGY_COST 0
#define SPECIAL_WAIT 0

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
		dx = dx * 1/2;
		dy = dy * 1/2;

		// Add some of the Blade's velocity to its projectiles.
		DeltaVelocityComponents (&MissilePtr->velocity, dx, dy);
		MissilePtr->current.location.x -= VELOCITY_TO_WORLD (dx);
		MissilePtr->current.location.y -= VELOCITY_TO_WORLD (dy);

		UnlockElement (GobArray[0]);
	}
	
	return (1);
}

// spawn_ion_trail is a replacement for Supox exhaust particles.
// A special exception in ship.c has been made to disable default exhaust for Supox.
static void
spawn_ion_trail (ELEMENT *ElementPtr)
{
#define START_ION_COLOR BUILD_COLOR (MAKE_RGB15 (0x1F, 0x17, 0x00), 0x7a)

	if (ElementPtr->state_flags & PLAYER_SHIP)
	{
		HELEMENT hIonElement;

		hIonElement = AllocElement ();
		if (hIonElement)
		{
			ELEMENT *IonElementPtr;
			STARSHIP *StarShipPtr;

			GetElementStarShip (ElementPtr, &StarShipPtr);

			InsertElement (hIonElement, GetHeadElement ());
			LockElement (hIonElement, &IonElementPtr);
			IonElementPtr->state_flags = APPEARING | FINITE_LIFE | NONSOLID;
			IonElementPtr->life_span = IonElementPtr->thrust_wait = 1;
			SetPrimType (&DisplayArray[IonElementPtr->PrimIndex], POINT_PRIM);
			SetPrimColor (&DisplayArray[IonElementPtr->PrimIndex], START_ION_COLOR);
			IonElementPtr->current.image.frame = DecFrameIndex (stars_in_space);
			IonElementPtr->current.image.farray = &stars_in_space;
			IonElementPtr->current.location = ElementPtr->current.location;
			IonElementPtr->death_func = spawn_ion_trail;
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

static void
supox_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;
	SIZE add_facing;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	
	add_facing = 0;
	
	// "Down" key is the new reverse thrust.
	if (StarShipPtr->cur_status_flags & DOWN)
		add_facing = ANGLE_TO_FACING (HALF_CIRCLE);
	
	if ((StarShipPtr->cur_status_flags & SPECIAL))
	{
		if (StarShipPtr->cur_status_flags & LEFT)
		{
			if (ElementPtr->turn_wait == 0)
				++ElementPtr->turn_wait;

			if (add_facing)
				add_facing += ANGLE_TO_FACING (OCTANT);
			else
				add_facing = -ANGLE_TO_FACING (QUADRANT);
		}
		else if (StarShipPtr->cur_status_flags & RIGHT)
		{
			if (ElementPtr->turn_wait == 0)
				++ElementPtr->turn_wait;

			if (add_facing)
				add_facing -= ANGLE_TO_FACING (OCTANT);
			else
				add_facing = ANGLE_TO_FACING (QUADRANT);
		}
	}
		
	// Exhaust trail for Supox.
	if ((StarShipPtr->cur_status_flags & (THRUST | DOWN)
			|| (StarShipPtr->cur_status_flags & SPECIAL
				&& StarShipPtr->cur_status_flags & (LEFT | RIGHT)))
			&& ElementPtr->thrust_wait == 0
			&& StarShipPtr->special_counter == 0)
	{
		spawn_ion_trail(ElementPtr);
		
		StarShipPtr->special_counter += 2;
		// Why do this? Because 0 delay exhaust looks ugly during lateral thrust.
	}

	// Apply alternate thrust.
	if (add_facing && ElementPtr->thrust_wait == 0)
	{
		COUNT facing;
		STATUS_FLAGS thrust_status;

		facing = StarShipPtr->ShipFacing;
		StarShipPtr->ShipFacing = NORMALIZE_FACING (
				facing + add_facing
				);
		thrust_status = inertial_thrust (ElementPtr);
		StarShipPtr->cur_status_flags &=
				~(SHIP_AT_MAX_SPEED
				| SHIP_BEYOND_MAX_SPEED
				| SHIP_IN_GRAVITY_WELL);
		StarShipPtr->cur_status_flags |= thrust_status;
		StarShipPtr->ShipFacing = facing;
	}
}

static void
supox_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern, COUNT ConcernCounter)
{
	STARSHIP *StarShipPtr;
	EVALUATE_DESC *lpEvalDesc;

	GetElementStarShip (ShipPtr, &StarShipPtr);

	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
	if (StarShipPtr->special_counter || lpEvalDesc->ObjectPtr == 0)
		StarShipPtr->ship_input_state &= ~SPECIAL;
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
			StarShipPtr->ship_input_state &= ~SPECIAL;
		else if (LinedUp && lpEvalDesc->which_turn <= 12)
			StarShipPtr->ship_input_state |= SPECIAL;

		if (StarShipPtr->ship_input_state & SPECIAL)
			lpEvalDesc->MoveState = PURSUE;
	}

	ship_intelligence (ShipPtr,
			ObjectsOfConcern, ConcernCounter);

	if (StarShipPtr->ship_input_state & SPECIAL)
		StarShipPtr->ship_input_state |= THRUST | WEAPON;

	lpEvalDesc = &ObjectsOfConcern[ENEMY_WEAPON_INDEX];
	if (StarShipPtr->special_counter == 0
			&& lpEvalDesc->ObjectPtr
			&& lpEvalDesc->MoveState == AVOID
			&& ShipPtr->turn_wait == 0)
	{
		StarShipPtr->ship_input_state &= ~THRUST;
		StarShipPtr->ship_input_state |= SPECIAL;
		if (!(StarShipPtr->cur_status_flags & (LEFT | RIGHT)))
			StarShipPtr->ship_input_state |= 1 << ((BYTE)TFB_Random () & 1);
		else
			StarShipPtr->ship_input_state |=
					StarShipPtr->cur_status_flags & (LEFT | RIGHT);
	}
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

