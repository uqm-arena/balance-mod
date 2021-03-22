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
#include "arilou.h"
#include "resinst.h"
#include "uqm/colors.h"
#include "uqm/globdata.h"
#include "libs/mathlib.h"

// Core Characteristics
#define MAX_CREW 6
#define MAX_ENERGY 20
#define ENERGY_REGENERATION 1
#define ENERGY_WAIT 6
#define MAX_THRUST 42
#define THRUST_INCREMENT MAX_THRUST
#define THRUST_WAIT 0
#define TURN_WAIT 0
#define SHIP_MASS 1

// Tracking Laser
#define WEAPON_ENERGY_COST 2
#define WEAPON_WAIT 1
#define ARILOU_OFFSET 9
#define LASER_RANGE DISPLAY_TO_WORLD (90 + ARILOU_OFFSET)

// Teleporter
#define SPECIAL_ENERGY_COST 3
#define SPECIAL_WAIT 0
#define TRANSIT_TIME 14
#define REPOSITION_DELAY 5
#define SPECIAL_MOVEMENT_FREEZE 3
#define SPECIAL_LASER_FREEZE 12
#define PERSONAL_SPACE_BUBBLE		DISPLAY_TO_WORLD (400)
#define HOP_DISTANCE				DISPLAY_TO_WORLD (280)
#define HOP_VARIATION				DISPLAY_TO_WORLD (30)

static RACE_DESC arilou_desc =
{
	{ /* SHIP_INFO */
		/* FIRES_FORE | */ IMMEDIATE_WEAPON,
		16, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		ARILOU_RACE_STRINGS,
		ARILOU_ICON_MASK_PMAP_ANIM,
		ARILOU_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		250 / SPHERE_RADIUS_INCREMENT * 2, /* Initial SoI radius */
		{ /* Known location (center of SoI) */
			438, 6372,
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
			ARILOU_BIG_MASK_PMAP_ANIM,
			ARILOU_MED_MASK_PMAP_ANIM,
			ARILOU_SML_MASK_PMAP_ANIM,
		},
		{
			NULL_RESOURCE,
			NULL_RESOURCE,
			NULL_RESOURCE,
		},
		{
			WARP_BIG_MASK_PMAP_ANIM,
			WARP_MED_MASK_PMAP_ANIM,
			WARP_SML_MASK_PMAP_ANIM,
		},
		{
			ARILOU_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		ARILOU_VICTORY_SONG,
		ARILOU_SHIP_SOUNDS,
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		NULL, NULL
	},
	{
		0,
		LASER_RANGE >> 1,
		NULL,
	},
	(UNINIT_FUNC *) NULL,
	(PREPROCESS_FUNC *) NULL,
	(POSTPROCESS_FUNC *) NULL,
	(INIT_WEAPON_FUNC *) NULL,
	0,
	0, /* CodeRef */
};

SIZE
LaserAim (ELEMENT *Tracker, COUNT *pAngle)
{
	SIZE best_delta_angle, best_delta;
	HELEMENT hShip, hNextShip;
	ELEMENT *Trackee;

	best_delta = 0;
	best_delta_angle = -1;

	hShip = Tracker->hTarget;
	if (hShip)
	{
		LockElement (hShip, &Trackee);
		Tracker->hTarget = hNextShip = 0;

		goto CheckTracking;
	}

	for (hShip = GetHeadElement (); hShip != 0; hShip = hNextShip)
	{
		LockElement (hShip, &Trackee);
		hNextShip = GetSuccElement (Trackee);
		// a & !b & (!c | (d & e))
		if ((Trackee->state_flags & PLAYER_SHIP)
				&& !elementsOfSamePlayer (Trackee, Tracker)
				&& (!OBJECT_CLOAKED (Trackee)
				|| ((Tracker->state_flags & PLAYER_SHIP)
				&& (Tracker->state_flags & APPEARING))
				))
		{
			STARSHIP *StarShipPtr;

CheckTracking:
			GetElementStarShip (Trackee, &StarShipPtr);
			if (Trackee->life_span
					&& StarShipPtr->RaceDescPtr->ship_info.crew_level)
			{
				SIZE delta_x, delta_y, delta_angle;

				if (Tracker->state_flags & PRE_PROCESS)
				{
					delta_x = Trackee->next.location.x
							- Tracker->next.location.x;
					delta_y = Trackee->next.location.y
							- Tracker->next.location.y;
				}
				else
				{
					delta_x = Trackee->current.location.x
							- Tracker->current.location.x;
					delta_y = Trackee->current.location.y
							- Tracker->current.location.y;
				}

				delta_x = WRAP_DELTA_X (delta_x);
				delta_y = WRAP_DELTA_Y (delta_y);
				delta_angle = NORMALIZE_ANGLE ((ARCTAN (delta_x, delta_y)) - *pAngle);

				if (delta_x < 0)
					delta_x = -delta_x;
				if (delta_y < 0)
					delta_y = -delta_y;
				delta_x += delta_y;

				if (best_delta == 0 || delta_x < best_delta)
				{
					best_delta = delta_x;
					best_delta_angle = delta_angle;
					Tracker->hTarget = hShip;
				}
			}
		}
		UnlockElement (hShip);
	}

	if (best_delta_angle > 0)
	{
		COUNT angle;

		angle = *pAngle;
		if (best_delta_angle == HALF_CIRCLE)
			angle += (((BYTE)TFB_Random () & 1) << 1) - 1;
		else if (best_delta_angle < ANGLE_TO_FACING (HALF_CIRCLE))
			++angle;
		else
			--angle;
		*pAngle = NORMALIZE_ANGLE (angle);
	}

	return (best_delta_angle);
}

static COUNT
initialize_autoaim_laser (ELEMENT *ShipPtr, HELEMENT LaserArray[])
{
	SIZE ship_angle, delta_angle, attack_angle;
	STARSHIP *StarShipPtr;
	LASER_BLOCK LaserBlock;

	GetElementStarShip (ShipPtr, &StarShipPtr);

	attack_angle = ship_angle = FACING_TO_ANGLE (StarShipPtr->ShipFacing);

	if ((delta_angle = LaserAim (ShipPtr, &ship_angle)) > 0)
	{
		attack_angle = NORMALIZE_ANGLE (ship_angle + delta_angle);
	}
	ShipPtr->hTarget = 0;

	LaserBlock.face = ANGLE_TO_FACING (attack_angle);
	LaserBlock.cx = ShipPtr->next.location.x;
	LaserBlock.cy = ShipPtr->next.location.y;
	LaserBlock.ex = COSINE (attack_angle, LASER_RANGE);
	LaserBlock.ey = SINE (attack_angle, LASER_RANGE);
	LaserBlock.sender = ShipPtr->playerNr;
	LaserBlock.flags = IGNORE_SIMILAR;
	LaserBlock.pixoffs = ARILOU_OFFSET;
	LaserBlock.color = BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x0A), 0x0E);
	LaserArray[0] = initialize_laser (&LaserBlock);

	return (1);
}

static void
arilou_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;
	COUNT facing;
	PRIMITIVE *lpPrim;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	lpPrim = &(GLOBAL (DisplayArray))[ElementPtr->PrimIndex];
	facing = StarShipPtr->ShipFacing;
	
	// Stop immediately when thrusters are off
	if (ElementPtr->thrust_wait == 0)
	{
		ZeroVelocityComponents (&ElementPtr->velocity);
		StarShipPtr->cur_status_flags &= ~SHIP_AT_MAX_SPEED;
	}
	
	if (!(ElementPtr->state_flags & NONSOLID))
	{	
		if ((StarShipPtr->cur_status_flags & SPECIAL)
				&& StarShipPtr->special_counter == 0
				&& DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST))
		{
			// Initiate teleport
			ZeroVelocityComponents (&ElementPtr->velocity);

			ElementPtr->state_flags |= NONSOLID | FINITE_LIFE | CHANGING;
			ElementPtr->life_span = TRANSIT_TIME;
			ElementPtr->is_teleporting = TRUE;

			ElementPtr->next.image.farray =	StarShipPtr->RaceDescPtr->ship_data.special;
			ElementPtr->next.image.frame = StarShipPtr->RaceDescPtr->ship_data.special[0];

			// Remember thrust key
			if (StarShipPtr->cur_status_flags & THRUST)
			{
				StarShipPtr->cur_status_flags &=
					~(SHIP_AT_MAX_SPEED | LEFT | RIGHT | DOWN | WEAPON);

				StarShipPtr->old_status_flags |= THRUST;
			}
			else
			{
				StarShipPtr->cur_status_flags &=
					~(SHIP_AT_MAX_SPEED | LEFT | RIGHT | THRUST | DOWN | WEAPON);

				StarShipPtr->old_status_flags &= ~(THRUST);
			}
			
			ProcessSound (SetAbsSoundIndex (
							/* HYPERJUMP */
					StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);
		}
	}
	else if (ElementPtr->next.image.farray == StarShipPtr->RaceDescPtr->ship_data.special)
	{
		HELEMENT hWarpElement;
		COUNT life_span;

		// Remember thrust key
		if (StarShipPtr->old_status_flags & THRUST)
		{
			StarShipPtr->cur_status_flags =
				(StarShipPtr->cur_status_flags
				& ~(LEFT | RIGHT | DOWN | WEAPON))
				| (StarShipPtr->old_status_flags
				& (LEFT | RIGHT | DOWN | WEAPON));

			StarShipPtr->cur_status_flags |= THRUST;
			StarShipPtr->old_status_flags |= THRUST;
		}
		else
		{
			StarShipPtr->cur_status_flags =
				(StarShipPtr->cur_status_flags
				& ~(LEFT | RIGHT | THRUST | DOWN | WEAPON))
				| (StarShipPtr->old_status_flags
				& (LEFT | RIGHT | THRUST | DOWN | WEAPON));
		}

		++StarShipPtr->weapon_counter;
		++StarShipPtr->special_counter;
		++StarShipPtr->energy_counter;
		++ElementPtr->turn_wait;
		++ElementPtr->thrust_wait;

		// End of teleport sequence
		if ((life_span = ElementPtr->life_span) == NORMAL_LIFE)
		{
			ElementPtr->current.image.farray =
				ElementPtr->next.image.farray =
					StarShipPtr->RaceDescPtr->ship_data.ship;
			ElementPtr->current.image.frame =
				ElementPtr->next.image.frame =
					SetAbsFrameIndex (StarShipPtr->RaceDescPtr->ship_data.ship[0],
					StarShipPtr->ShipFacing);

			// Teleport again if the special button is held down
			if (StarShipPtr->cur_status_flags & SPECIAL
				&& StarShipPtr->RaceDescPtr->ship_info.energy_level >= SPECIAL_ENERGY_COST)
			{
				ElementPtr->life_span = TRANSIT_TIME;

				ElementPtr->current.image.farray =
					ElementPtr->next.image.farray =
						StarShipPtr->RaceDescPtr->ship_data.special;
				ElementPtr->current.image.frame =
					ElementPtr->next.image.frame =
						SetAbsFrameIndex (ElementPtr->next.image.frame, 0);

				DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST);

				ProcessSound (SetAbsSoundIndex (
						/* HYPERJUMP */
					StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);
			}
			// Prevent the Skiff from spawning inside solid matter
			else if (Overlap (ElementPtr)
				// && StarShipPtr->RaceDescPtr->ship_info.crew_level > 1
				&& StarShipPtr->RaceDescPtr->ship_info.energy_level >= SPECIAL_ENERGY_COST)
			{
				// Don't forward-teleport again.
				StarShipPtr->cur_status_flags &= ~THRUST;
				StarShipPtr->old_status_flags &= ~THRUST;

				ElementPtr->life_span = TRANSIT_TIME;

				ElementPtr->current.image.farray =
					ElementPtr->next.image.farray =
						StarShipPtr->RaceDescPtr->ship_data.special;
				ElementPtr->current.image.frame =
					ElementPtr->next.image.frame =
						SetAbsFrameIndex (ElementPtr->next.image.frame, 0);

				DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST);
				// DeltaCrew (ElementPtr, -1); // Inflict damage whenever Arilou escapes overlap

				ProcessSound (SetAbsSoundIndex (
						/* HYPERJUMP */
					StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);
			}
			else
			{
				// Reappear
				ElementPtr->state_flags &= ~(NONSOLID | FINITE_LIFE);
				ElementPtr->state_flags |= APPEARING;
				InitIntersectStartPoint (ElementPtr);

				// Stun the Skiff for an instant
				StarShipPtr->weapon_counter = SPECIAL_LASER_FREEZE;
				ElementPtr->thrust_wait = SPECIAL_MOVEMENT_FREEZE;
				ElementPtr->turn_wait = SPECIAL_MOVEMENT_FREEZE;
				
				ElementPtr->is_teleporting = FALSE;
			}
		}
		else
		{
			// Teleportation in progress
			--life_span;
			if (life_span != TRANSIT_TIME - REPOSITION_DELAY)
				{
					if (life_span == 1)
					{
						// Image frame 0 is portal image #1
						ElementPtr->next.image.frame =
							SetAbsFrameIndex (ElementPtr->next.image.frame, 0);
					}
					else if (life_span >= TRANSIT_TIME - 2 || life_span <= 3)
					{
						// Image frame 1 is portal image #2
						ElementPtr->next.image.frame =
							SetAbsFrameIndex (ElementPtr->next.image.frame, 1);
					}
					else
					{
						// Image frame 2 should be a blank frame
						ElementPtr->next.image.frame =
							SetAbsFrameIndex (ElementPtr->next.image.frame, 2);
					}
				}
			else
			{
				SIZE dx, dy;
				ELEMENT *OtherShipPtr;
				
				// Forward teleport
				if (StarShipPtr->old_status_flags & THRUST)
				{
					ElementPtr->next.location.x =
						WRAP_X (ElementPtr->current.location.x
						+ (COSINE (FACING_TO_ANGLE (StarShipPtr->ShipFacing),
						(HOP_DISTANCE + HOP_VARIATION - (TFB_Random () % HOP_VARIATION << 1) ) ) )
						+ (COSINE (FACING_TO_ANGLE (StarShipPtr->ShipFacing + 4),
						HOP_VARIATION - (TFB_Random () % HOP_VARIATION << 1) ) ) );

					ElementPtr->next.location.y =
						WRAP_Y (ElementPtr->current.location.y
						+ (SINE (FACING_TO_ANGLE (StarShipPtr->ShipFacing),
						(HOP_DISTANCE + HOP_VARIATION - (TFB_Random () % HOP_VARIATION << 1) ) ) )
						+ (SINE (FACING_TO_ANGLE (StarShipPtr->ShipFacing + 4),
						HOP_VARIATION - (TFB_Random () % HOP_VARIATION << 1) ) ) );
				}
				// Safe teleport
				else if (TrackAnyShip (ElementPtr, &facing) >= 0)
				{
					LockElement (ElementPtr->hTarget, &OtherShipPtr);
				
					do {
					//
						ElementPtr->next.location.x =
							WRAP_X (DISPLAY_ALIGN_X (TFB_Random () ));
						ElementPtr->next.location.y =
							WRAP_Y (DISPLAY_ALIGN_Y (TFB_Random () ));
					
						dx = WRAP_DELTA_X(OtherShipPtr->next.location.x - ElementPtr->next.location.x);
						dy = WRAP_DELTA_Y(OtherShipPtr->next.location.y - ElementPtr->next.location.y);
					//
					} while ((long)dx*dx + (long)dy*dy <=
						(long)PERSONAL_SPACE_BUBBLE * PERSONAL_SPACE_BUBBLE);

					UnlockElement (ElementPtr->hTarget);
				}
				// Teleport to anywhere if there is no opponent present
				else
				{
					ElementPtr->next.location.x =
						WRAP_X (DISPLAY_ALIGN_X (TFB_Random ()));
					ElementPtr->next.location.y =
						WRAP_Y (DISPLAY_ALIGN_Y (TFB_Random ()));
				}
			}
		}
		
		// Portal image stays on top of other images
		hWarpElement = AllocElement ();
		if (hWarpElement)
		{
			ELEMENT *WarpPtr;

			LockElement (hWarpElement, &WarpPtr);
			WarpPtr->state_flags = APPEARING | FINITE_LIFE | IGNORE_SIMILAR | NONSOLID
					| (ElementPtr->state_flags & (GOOD_GUY | BAD_GUY));
			SetPrimType (&(GLOBAL (DisplayArray))[WarpPtr->PrimIndex], GetPrimType (lpPrim));
			WarpPtr->current.location = ElementPtr->next.location;
			WarpPtr->current.image.farray = ElementPtr->next.image.farray;
			WarpPtr->current.image.frame = ElementPtr->next.image.frame;
			WarpPtr->life_span = 1;
			SetElementStarShip (WarpPtr, StarShipPtr);
			UnlockElement (hWarpElement);
			InsertElement (hWarpElement, GetTailElement ());
		}

		ElementPtr->state_flags |= CHANGING;
	}
}

static void
arilou_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern,
		COUNT ConcernCounter)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	StarShipPtr->ship_input_state |= THRUST;

	 ObjectsOfConcern[ENEMY_SHIP_INDEX].MoveState = ENTICE;
	ship_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter);

	if (StarShipPtr->special_counter == 0)
	{
		EVALUATE_DESC *lpEvalDesc;

		StarShipPtr->ship_input_state &= ~SPECIAL;

		lpEvalDesc = &ObjectsOfConcern[ENEMY_WEAPON_INDEX];
		if (lpEvalDesc->ObjectPtr && lpEvalDesc->which_turn <= 6)
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
					|| (IsTrackingWeapon && (lpEvalDesc->which_turn == 1
					|| (lpEvalDesc->ObjectPtr->state_flags & CREW_OBJECT))) /* FIGHTERS!!! */
					|| PlotIntercept (lpEvalDesc->ObjectPtr, ShipPtr, 3, 0))
					&& !(TFB_Random () & 3))
			{
				StarShipPtr->ship_input_state &= ~(LEFT | RIGHT | THRUST | WEAPON);
				StarShipPtr->ship_input_state |= SPECIAL;
			}
		}
	}

	if (StarShipPtr->RaceDescPtr->ship_info.energy_level <= SPECIAL_ENERGY_COST << 1)
		StarShipPtr->ship_input_state &= ~WEAPON;
}

RACE_DESC*
init_arilou (void)
{
	RACE_DESC *RaceDescPtr;

	arilou_desc.preprocess_func = arilou_preprocess;
	arilou_desc.init_weapon_func = initialize_autoaim_laser;
	arilou_desc.cyborg_control.intelligence_func = arilou_intelligence;

	RaceDescPtr = &arilou_desc;

	return (RaceDescPtr);
}

