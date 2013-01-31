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
#include "urquan.h"
#include "resinst.h"
#include "../../colors.h"
#include "libs/mathlib.h"
#include "uqm/globdata.h"
#include <stdlib.h>

// Core characteristics
#define MAX_CREW 42
#define MAX_ENERGY 42
#define ENERGY_REGENERATION 1
#define ENERGY_WAIT 4
#define MAX_THRUST 30
#define THRUST_INCREMENT 6
#define THRUST_WAIT 6
#define TURN_WAIT 4
#define SHIP_MASS 9

// Fusion cannons
#define WEAPON_ENERGY_COST 5
#define WEAPON_WAIT 6
#define CENTER_OFFSET 34
#define SIDE_OFFSET 23
#define MISSILE_SPEED 80
#define MISSILE_LIFE 20
#define MISSILE_HITS 10
#define MISSILE_DAMAGE 6
#define MISSILE_OFFSET 8

// Fighters
#define SPECIAL_ENERGY_COST 8
#define SPECIAL_WAIT 9
#define URQUAN_OFFSET 32
#define FIGHTER_OFFSET 4
#define FIGHTER_SPEED 32
#define FIGHTER_LIFE 680
#define FIGHTER_LAUNCH_TIME 6
#define FIGHTER_PURSUIT_TIME 275
#define FIGHTER_RETURN_TIME (FIGHTER_LIFE - FIGHTER_PURSUIT_TIME)
#define FIGHTER_HITS 1
#define FIGHTER_MASS 0
#define FIGHTER_WEAPON_WAIT 24
#define FIGHTER_LASER_RANGE (48 + FIGHTER_OFFSET)

// Auto-Turret
#define AUXILIARY_ENERGY_COST 1
#define AUXILIARY_WAIT 22
#define TURRET_OFFSET 7
#define TURRET_MISSILE_OFFSET 4
#define TURRET_MISSILE_SPEED 80
#define TURRET_MISSILE_LIFE 10
#define TURRET_MISSILE_HITS 1
#define TURRET_MISSILE_DAMAGE 1
#define TURRET_RANGE DISPLAY_TO_WORLD(160)
#define MAX_TARGET_HITS 3

static RACE_DESC urquan_desc =
{
	{ /* SHIP_INFO */
		FIRES_FORE | SEEKING_SPECIAL,
		30, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		URQUAN_RACE_STRINGS,
		URQUAN_ICON_MASK_PMAP_ANIM,
		URQUAN_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		2666 / SPHERE_RADIUS_INCREMENT * 2, /* Initial SoI radius */
		{ /* Known location (center of SoI) */
			5750, 6000,
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
			URQUAN_BIG_MASK_PMAP_ANIM,
			URQUAN_MED_MASK_PMAP_ANIM,
			URQUAN_SML_MASK_PMAP_ANIM,
		},
		{
			FUSION_BIG_MASK_PMAP_ANIM,
			FUSION_MED_MASK_PMAP_ANIM,
			FUSION_SML_MASK_PMAP_ANIM,
		},
		{
			SPECIAL_BIG_MASK_PMAP_ANIM,
			SPECIAL_MED_MASK_PMAP_ANIM,
			SPECIAL_SML_MASK_PMAP_ANIM,
		},
		{
			URQUAN_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		URQUAN_VICTORY_SONG,
		URQUAN_SHIP_SOUNDS,
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		NULL, NULL
	},
	{
		0,
		MISSILE_SPEED * MISSILE_LIFE,
		NULL,
	},
	(UNINIT_FUNC *) NULL,
	(PREPROCESS_FUNC *) NULL,
	(POSTPROCESS_FUNC *) NULL,
	(INIT_WEAPON_FUNC *) NULL,
	0,
	0, /* CodeRef */
};

// Prevent fusion bolts from killing fighters that are on a return trip to the Dreadnought.
static void
fusion_collision (ELEMENT *ElementPtr0, POINT *pPt0, ELEMENT *ElementPtr1, POINT *pPt1)
{
    if (ElementPtr0->pParent != ElementPtr1->pParent)
 		weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
}

static COUNT
initialize_fusion (ELEMENT *ShipPtr, HELEMENT FusionArray[])
{
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
	MissileBlock.preprocess_func = NULL;
	MissileBlock.blast_offs = MISSILE_OFFSET;
    

	if (StarShipPtr->static_counter == 1)
	{
		MissileBlock.pixoffs = SIDE_OFFSET;
		MissileBlock.cx = ShipPtr->next.location.x
			+ COSINE(FACING_TO_ANGLE (MissileBlock.face + 4), -48);
		MissileBlock.cy = ShipPtr->next.location.y
			+ SINE(FACING_TO_ANGLE (MissileBlock.face + 4), -48);
	}
	else if (StarShipPtr->static_counter == 3)
	{
		MissileBlock.pixoffs = SIDE_OFFSET;
		MissileBlock.cx = ShipPtr->next.location.x
			+ COSINE(FACING_TO_ANGLE (MissileBlock.face + 4), 48);
		MissileBlock.cy = ShipPtr->next.location.y
			+ SINE(FACING_TO_ANGLE (MissileBlock.face + 4), 48);
	}
	else
	{
		MissileBlock.pixoffs = CENTER_OFFSET;
		MissileBlock.cx = ShipPtr->next.location.x;
		MissileBlock.cy = ShipPtr->next.location.y;
	}
    
	FusionArray[0] = initialize_missile (&MissileBlock);
    
	if (FusionArray[0])
	{
		SIZE dx, dy;
		ELEMENT *FusionPtr;

		LockElement (FusionArray[0], &FusionPtr);

		GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
		DeltaVelocityComponents (&FusionPtr->velocity, dx, dy);
		FusionPtr->current.location.x -= VELOCITY_TO_WORLD (dx);
		FusionPtr->current.location.y -= VELOCITY_TO_WORLD (dy);
		FusionPtr->collision_func = fusion_collision;
		
		UnlockElement (FusionArray[0]);
	}
    
	return (1);
}

// Currently the fighter beam removes 1 crew and 1 energy per shot.
static void
laser_collision (ELEMENT *ElementPtr0, POINT *pPt0, ELEMENT *ElementPtr1, POINT *pPt1)
{
	STARSHIP *StarShipPtr, *EnemyStarShipPtr;
	HELEMENT hBlastElement;
	
	GetElementStarShip (ElementPtr0, &StarShipPtr);
	GetElementStarShip (ElementPtr1, &EnemyStarShipPtr);
	
	if (EnemyStarShipPtr && ElementPtr1->state_flags & PLAYER_SHIP)
	{
		if (EnemyStarShipPtr->RaceDescPtr->ship_info.energy_level > 0)
			DeltaEnergy (ElementPtr1, -1);
		else
			// Reset target's energy_counter if there's no energy left.
			DeltaEnergy (ElementPtr1, -EnemyStarShipPtr->RaceDescPtr->ship_info.energy_level);

		// Shielded targets do not take damage.
		if (ElementPtr1->life_span == NORMAL_LIFE)
		{
			if (ElementPtr1->crew_level > 1)
				DeltaCrew (ElementPtr1, -1);
			else if (ElementPtr1->crew_level == 1)	
			{
				DeltaCrew (ElementPtr1, -1);
				ElementPtr1->life_span = 0;
			}
		}
	}

	hBlastElement = weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);

	if (hBlastElement)
	{
		ELEMENT *BlastElementPtr;

		LockElement (hBlastElement, &BlastElementPtr);

		BlastElementPtr->life_span = 2;
		BlastElementPtr->current.image.farray = StarShipPtr->RaceDescPtr->ship_data.special;
		BlastElementPtr->current.image.frame =
			SetAbsFrameIndex (BlastElementPtr->current.image.farray[0], 16);
		BlastElementPtr->preprocess_func = NULL;

		UnlockElement (hBlastElement);
	}
}

static void
spawn_fighter_laser (ELEMENT *ElementPtr)
{
	//UWORD best_dist;
	STARSHIP *StarShipPtr;
	HELEMENT hObject, hNextObject, hBestObject;
	ELEMENT *ShipPtr, *FighterPtr, *ObjectPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	LockElement (StarShipPtr->hShip, &ShipPtr);
	LockElement (ElementPtr->hTarget, &FighterPtr);
	hBestObject = 0;
	//best_dist = FIGHTER_LASER_RANGE + 1;

	for (hObject = GetPredElement (ElementPtr);
			hObject; hObject = hNextObject)
	{
		LockElement (hObject, &ObjectPtr);
		hNextObject = GetPredElement (ObjectPtr);

		if (!elementsOfSamePlayer (ObjectPtr, ShipPtr)
				&& CollisionPossible (ObjectPtr, ShipPtr)
				&& ObjectPtr->state_flags & PLAYER_SHIP
				&& !OBJECT_CLOAKED (ObjectPtr))
		{
			SIZE delta_x, delta_y;
			long dist;

			delta_x = ObjectPtr->next.location.x
					- FighterPtr->next.location.x;
			delta_y = ObjectPtr->next.location.y
					- FighterPtr->next.location.y;

			if (delta_x < 0)
				delta_x = -delta_x;
			if (delta_y < 0)
				delta_y = -delta_y;

			delta_x = WORLD_TO_DISPLAY (delta_x);
			delta_y = WORLD_TO_DISPLAY (delta_y);

			if (delta_x <= FIGHTER_LASER_RANGE
				&& delta_y <= FIGHTER_LASER_RANGE
				&& (dist = (long)delta_x * delta_x + (long)delta_y * delta_y)
				<= (long)FIGHTER_LASER_RANGE * FIGHTER_LASER_RANGE)
			{
				hBestObject = hObject;
				//best_dist = dist;
			}
		}
		UnlockElement (hObject);
	}

	if (hBestObject)
	{
		LASER_BLOCK LaserBlock;
		HELEMENT hPointDefense;

		LockElement (hBestObject, &ObjectPtr);

		LaserBlock.cx = FighterPtr->next.location.x;
		LaserBlock.cy = FighterPtr->next.location.y;
		LaserBlock.face = 0;

		//Omni-directional laser:
		LaserBlock.ex = ObjectPtr->next.location.x
				- FighterPtr->next.location.x;
		LaserBlock.ey = ObjectPtr->next.location.y
				- FighterPtr->next.location.y;

		LaserBlock.sender = FighterPtr->playerNr;
		LaserBlock.flags = IGNORE_SIMILAR;		
		LaserBlock.pixoffs = FIGHTER_OFFSET;
		LaserBlock.color = BUILD_COLOR (MAKE_RGB15 (0x05, 0x16, 0x1F), 0x00);
		hPointDefense = initialize_laser (&LaserBlock);
		
		if (hPointDefense)
		{
			ELEMENT *PDPtr;

			LockElement (hPointDefense, &PDPtr);
			SetElementStarShip (PDPtr, StarShipPtr);
			PDPtr->hTarget = 0;	
			PDPtr->mass_points = 0; // Damage is determined elsewhere.
			PDPtr->collision_func = laser_collision;

			ProcessSound (SetAbsSoundIndex
					/* FIGHTER_ZAP */
				(StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 3), PDPtr);

			UnlockElement (hPointDefense);

			PutElement (hPointDefense);

			FighterPtr->thrust_wait = FIGHTER_WEAPON_WAIT;
		}
		
		UnlockElement (hBestObject);
	}
	
	UnlockElement (ElementPtr->hTarget);
	UnlockElement (StarShipPtr->hShip);
}

static void
fighter_postprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	// Decrement weapon cooldown.
	if (ElementPtr->thrust_wait > 0)
		--ElementPtr->thrust_wait;
	
	// Spawn_fighter_laser if the laser is ready.
	if (ElementPtr->thrust_wait == 0)
	{
		HELEMENT hPew;

		hPew = AllocElement ();
		if (hPew)
		{
			ELEMENT *PewPtr;
			
			PutElement (hPew);

			LockElement (hPew, &PewPtr);
			PewPtr->state_flags = APPEARING | NONSOLID | FINITE_LIFE
					| (ElementPtr->state_flags & (GOOD_GUY | BAD_GUY));
			{
				ELEMENT *SuccPtr;

				LockElement (GetSuccElement (ElementPtr), &SuccPtr);
				PewPtr->hTarget = GetPredElement (SuccPtr);
				UnlockElement (GetSuccElement (ElementPtr));
				
				PewPtr->death_func = spawn_fighter_laser;
			}

			GetElementStarShip (ElementPtr, &StarShipPtr);
			SetElementStarShip (PewPtr, StarShipPtr);

			UnlockElement (hPew);
		}
	}
}

static void
fighter_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);

	// Keep a count of how many fighters are deployed.
	++StarShipPtr->RaceDescPtr->characteristics.special_wait;
	
	if ((FIGHTER_LIFE - ElementPtr->life_span) > FIGHTER_LAUNCH_TIME
		&& !(ElementPtr->state_flags & CHANGING))
	{
		BOOLEAN Enroute;
		COUNT fighter_facing;
		SIZE delta_x, delta_y, delta_facing=0, fighter_speed;
		ELEMENT *TargetPtr;
		HELEMENT hTarget;
		
		Enroute = TRUE;
		delta_x = StarShipPtr->RaceDescPtr->ship_info.crew_level;
		delta_y = ElementPtr->life_span;
        fighter_speed = FIGHTER_SPEED; // Use default speed for now.
		fighter_facing = GetFrameIndex (ElementPtr->current.image.frame);
		
		if (((delta_y & 1) || ElementPtr->hTarget
				|| TrackShip (ElementPtr, &fighter_facing) >= 0)
				&& (delta_x == 0 || delta_y >= FIGHTER_RETURN_TIME))
			ElementPtr->state_flags |= IGNORE_SIMILAR;
		else if (delta_x)
		{
			// Return to Dreadnought.
			LockElement (StarShipPtr->hShip, &TargetPtr);
			delta_x = WRAP_DELTA_X (TargetPtr->current.location.x - ElementPtr->current.location.x);
			delta_y = WRAP_DELTA_Y (TargetPtr->current.location.y - ElementPtr->current.location.y);	
			UnlockElement (StarShipPtr->hShip);

			delta_facing = NORMALIZE_FACING (ANGLE_TO_FACING (
				ARCTAN (delta_x, delta_y)) - fighter_facing
				);
			
			ElementPtr->state_flags &= ~IGNORE_SIMILAR;
			Enroute = FALSE;
		}
        
		if (Enroute)
		{
			if (ElementPtr->hTarget == 0)
				hTarget = StarShipPtr->hShip;
			else
				hTarget = ElementPtr->hTarget;
			
			if (hTarget)
			{
				SIZE test_facing, target_speed;

				LockElement (hTarget, &TargetPtr);
				delta_x = WRAP_DELTA_X (TargetPtr->current.location.x
					- ElementPtr->current.location.x);
				delta_y = WRAP_DELTA_Y (TargetPtr->current.location.y
					- ElementPtr->current.location.y);

				test_facing = GetFrameIndex (TargetPtr->current.image.frame);
				if (ElementPtr->turn_wait & LEFT)
				{
					delta_x += COSINE (FACING_TO_ANGLE (test_facing - 5),
						DISPLAY_TO_WORLD (36));
					delta_y += SINE (FACING_TO_ANGLE (test_facing - 5),
						DISPLAY_TO_WORLD (36));
				}
				else
				{
					delta_x += COSINE (FACING_TO_ANGLE (test_facing + 5),
						DISPLAY_TO_WORLD (36));
					delta_y += SINE (FACING_TO_ANGLE (test_facing + 5),
						DISPLAY_TO_WORLD (36));
				}

				// Set default destination.
				delta_facing = NORMALIZE_FACING (ANGLE_TO_FACING (
					ARCTAN (delta_x, delta_y)) - fighter_facing);

				// Adjust flight path when shooting at a slow-ish moving enemy ship.
				if (ElementPtr->thrust_wait > 0)
				{
					GetCurrentVelocityComponents (&TargetPtr->velocity, &delta_x, &delta_y);

					test_facing = ANGLE_TO_FACING (ARCTAN (
						ElementPtr->current.location.x
						- TargetPtr->current.location.x,
						ElementPtr->current.location.y
						- TargetPtr->current.location.y));
					
					// Transform enemy velocity into cute little world units.
					target_speed = VELOCITY_TO_WORLD (square_root (VelocitySquared (delta_x, delta_y)));

					// Compare enemy facing to fighter position.
					test_facing = NORMALIZE_FACING (test_facing -
						GetFrameIndex (TargetPtr->current.image.frame));

					// 1) Is enemy speed within a certain threshold?
					// 2) Is the fighter behind the enemy's forward 135 degree arc?
					if (target_speed < FIGHTER_SPEED
						&& target_speed >= (FIGHTER_SPEED >> 1)
						&& test_facing >= ANGLE_TO_FACING (QUADRANT)
						&& test_facing <= ANGLE_TO_FACING (HALF_CIRCLE + QUADRANT))
					{
						// Match the target's speed. Variation added to reduce fighter clumping.
						fighter_speed = target_speed + (TFB_Random () & 6) - 2;

						// Do not exceed maximum fighter speed.
						if (fighter_speed > FIGHTER_SPEED)
							fighter_speed = FIGHTER_SPEED;

						// Match the target's flight path.
						delta_facing = NORMALIZE_FACING (ANGLE_TO_FACING (
							GetVelocityTravelAngle (&TargetPtr->velocity)) - fighter_facing);
					}
				}

				UnlockElement (hTarget);
			}
		}
        
		// Turn towards destination.
		if (delta_facing > 0)
		{
			if (delta_facing == ANGLE_TO_FACING (HALF_CIRCLE))
			{
				if (ElementPtr->turn_wait & LEFT)
					fighter_facing = NORMALIZE_FACING (fighter_facing - 1);
				else 
					fighter_facing = NORMALIZE_FACING (fighter_facing + 1);
			}
			else if (delta_facing > ANGLE_TO_FACING (HALF_CIRCLE))
				fighter_facing = NORMALIZE_FACING (fighter_facing - 1);
			else
				fighter_facing = NORMALIZE_FACING (fighter_facing + 1);
		}

		ElementPtr->state_flags |= CHANGING;

		ElementPtr->next.image.frame =
			SetAbsFrameIndex (ElementPtr->next.image.frame, fighter_facing);
		
		SetVelocityVector (&ElementPtr->velocity, fighter_speed, fighter_facing);

		// Fighters fan out when in close proximity to each other.
		if (ElementPtr->thrust_wait == 0)
		{
			long dist, best_dist;
			SIZE best_delta_x, best_delta_y;
			HELEMENT hObject, hNextObject;
			ELEMENT *ObjectPtr;

			best_dist = (FIGHTER_SPEED * FIGHTER_SPEED) * 2;
			best_delta_x = 0;

			// Check for nearby squadmates.
			for (hObject = GetHeadElement (); hObject; hObject = hNextObject)
			{
				LockElement (hObject, &ObjectPtr);
				hNextObject = GetSuccElement (ObjectPtr);
			
				// Is ObjectPtr a friendly fighter? Are its weapons offline?
				if (ObjectPtr->life_span
						&& elementsOfSamePlayer (ObjectPtr, ElementPtr)
						&& ObjectPtr->current.image.farray
								== StarShipPtr->RaceDescPtr->ship_data.special
						&& ObjectPtr->thrust_wait == 0
						&& !(ObjectPtr->state_flags & DISAPPEARING)
						&& ObjectPtr != ElementPtr)
				{
					delta_x = WRAP_DELTA_X (ObjectPtr->current.location.x
						- ElementPtr->current.location.x);
					delta_y = WRAP_DELTA_Y (ObjectPtr->current.location.y
						- ElementPtr->current.location.y);
					
					dist = (long)delta_x * delta_x + (long)delta_y * delta_y;

					// Distance check.
					if (dist < best_dist)
					{
						best_dist = dist;
						best_delta_x = delta_x;
						best_delta_y = delta_y;
					}
				}

				UnlockElement (hObject);
			}

			// Is there a squadmate nearby?
			if (best_delta_x)
			{
				long magnitude;

				magnitude = WORLD_TO_VELOCITY(12);
				best_dist = square_root (best_dist); 
				
				// Fan out.
				DeltaVelocityComponents (&ElementPtr->velocity,
					-(long)magnitude * best_delta_x / best_dist,
					-(long)magnitude * best_delta_y / best_dist);
			}
		}
	}
}

static void
fighter_collision (ELEMENT *ElementPtr0, POINT *pPt0, ELEMENT *ElementPtr1, POINT *pPt1)
{
	STARSHIP *StarShipPtr, *OtherShipPtr;

	GetElementStarShip (ElementPtr0, &StarShipPtr);
    GetElementStarShip (ElementPtr1, &OtherShipPtr);
	
	// Fighters dodge planets, asteroids, enemy capital ships and other Ur-Quan fighters.
	if (ElementPtr1->playerNr == NEUTRAL_PLAYER_NUM
		|| ((StarShipPtr && ElementPtr1->state_flags & PLAYER_SHIP)
			&& (ElementPtr0->playerNr != ElementPtr1->playerNr))
		|| ElementPtr1->preprocess_func == fighter_preprocess)
	{
		HELEMENT hFighterElement;

		hFighterElement = AllocElement ();
		if (hFighterElement)
		{
			COUNT primIndex, travel_facing;
			SIZE delta_facing;
			ELEMENT *FighterElementPtr;

			LockElement (hFighterElement, &FighterElementPtr);
			primIndex = FighterElementPtr->PrimIndex;
			*FighterElementPtr = *ElementPtr0;
			FighterElementPtr->PrimIndex = primIndex;
			(GLOBAL (DisplayArray))[primIndex] = (GLOBAL (DisplayArray))[ElementPtr0->PrimIndex];
			FighterElementPtr->state_flags &= ~PRE_PROCESS;
			FighterElementPtr->state_flags |= CHANGING;
			FighterElementPtr->next = FighterElementPtr->current;
			travel_facing = GetVelocityTravelAngle (&FighterElementPtr->velocity);
			delta_facing = NORMALIZE_ANGLE (ARCTAN (
				pPt1->x - pPt0->x, pPt1->y - pPt0->y) - travel_facing);

			if (delta_facing == 0)
			{
				if ((TFB_Random () & 1) == 0)
					travel_facing -= QUADRANT;
				else
					travel_facing += QUADRANT;
			}
			else if (delta_facing <= HALF_CIRCLE)
				travel_facing -= QUADRANT;
			else
				travel_facing += QUADRANT;

			travel_facing = NORMALIZE_FACING (ANGLE_TO_FACING (
				NORMALIZE_ANGLE (travel_facing)));
				
			FighterElementPtr->next.image.frame =
				SetAbsFrameIndex (FighterElementPtr->next.image.frame, travel_facing);
				
			SetVelocityVector (&FighterElementPtr->velocity, FIGHTER_SPEED, travel_facing);
			
			UnlockElement (hFighterElement);
			PutElement (hFighterElement);
		}

		ElementPtr0->state_flags |= DISAPPEARING | COLLISION;
	}
	else if (ElementPtr0->pParent != ElementPtr1->pParent)
	{
		HELEMENT hBlastElement;

		ElementPtr0->blast_offset = 0;
		hBlastElement = weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
		ElementPtr0->state_flags |= DISAPPEARING | COLLISION;

		// Fighter explodes when destroyed.
		if (hBlastElement)
		{
			ELEMENT *BlastElementPtr;

			LockElement (hBlastElement, &BlastElementPtr);

			BlastElementPtr->life_span = 2;
			BlastElementPtr->current.image.farray = ElementPtr0->next.image.farray;
			BlastElementPtr->current.image.frame =
				SetAbsFrameIndex (BlastElementPtr->current.image.farray[0], 17);
			BlastElementPtr->preprocess_func = NULL;

			UnlockElement (hBlastElement);
		}
	}
	else if (ElementPtr1->state_flags & PLAYER_SHIP)
	{
		ProcessSound (SetAbsSoundIndex
				/* FIGHTERS_RETURN */
			(StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 4), ElementPtr1);

		DeltaCrew (ElementPtr1, 1);
		ElementPtr0->state_flags |= DISAPPEARING | COLLISION;
	}

	if (ElementPtr0->state_flags & DISAPPEARING)
	{
		ElementPtr0->state_flags &= ~DISAPPEARING;

		ElementPtr0->hit_points = 0;
		ElementPtr0->life_span = 0;
		ElementPtr0->state_flags |= NONSOLID;
		
		--StarShipPtr->RaceDescPtr->characteristics.special_wait;
	}
}

static void
spawn_fighters (ELEMENT *ElementPtr)
{
	SIZE fighters_launching;
	COUNT facing;
	SIZE delta_x, delta_y;
	HELEMENT hFighterElement;
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	facing = StarShipPtr->ShipFacing + ANGLE_TO_FACING (HALF_CIRCLE);
	delta_x = COSINE (FACING_TO_ANGLE (facing), DISPLAY_TO_WORLD (14));
	delta_y = SINE (FACING_TO_ANGLE (facing), DISPLAY_TO_WORLD (14));

	fighters_launching = ElementPtr->crew_level > 2 ? 2 : 1;

	while (fighters_launching-- && (hFighterElement = AllocElement ()))
	{
		SIZE sx, sy;
		COUNT fighter_facing;
		ELEMENT *FighterElementPtr;

		DeltaCrew (ElementPtr, -1);

		PutElement (hFighterElement);
		LockElement (hFighterElement, &FighterElementPtr);
		FighterElementPtr->hit_points = FIGHTER_HITS;
		FighterElementPtr->mass_points = FIGHTER_MASS;
		FighterElementPtr->life_span = FIGHTER_LIFE;
        FighterElementPtr->thrust_wait = FIGHTER_WEAPON_WAIT;
		FighterElementPtr->playerNr = ElementPtr->playerNr;
		FighterElementPtr->state_flags = APPEARING | FINITE_LIFE
				| CREW_OBJECT | IGNORE_SIMILAR;
		SetPrimType (&(GLOBAL (DisplayArray))[FighterElementPtr->PrimIndex], STAMP_PRIM);
		FighterElementPtr->preprocess_func = fighter_preprocess;
		FighterElementPtr->postprocess_func = fighter_postprocess;
		FighterElementPtr->collision_func = fighter_collision;
		FighterElementPtr->death_func = NULL;

		FighterElementPtr->current.location = ElementPtr->next.location;
		if (fighters_launching == 1)
		{
			FighterElementPtr->turn_wait = 0;
			fighter_facing = NORMALIZE_FACING (facing + 2);
			FighterElementPtr->current.location.x += delta_x - delta_y;
			FighterElementPtr->current.location.y += delta_y + delta_x;
		}
		else
		{
			FighterElementPtr->turn_wait = 1;
			fighter_facing = NORMALIZE_FACING (facing - 2);
			FighterElementPtr->current.location.x += delta_x + delta_y;
			FighterElementPtr->current.location.y += delta_y - delta_x;
		}
		sx = COSINE (FACING_TO_ANGLE (fighter_facing),
				WORLD_TO_VELOCITY (FIGHTER_SPEED));
		sy = SINE (FACING_TO_ANGLE (fighter_facing),
				WORLD_TO_VELOCITY (FIGHTER_SPEED));
		SetVelocityComponents (&FighterElementPtr->velocity, sx, sy);
		FighterElementPtr->current.location.x -= VELOCITY_TO_WORLD (sx);
		FighterElementPtr->current.location.y -= VELOCITY_TO_WORLD (sy);

		FighterElementPtr->current.image.farray = StarShipPtr->RaceDescPtr->ship_data.special;
		FighterElementPtr->current.image.frame =
				SetAbsFrameIndex (StarShipPtr->RaceDescPtr->ship_data.special[0],
				fighter_facing);
		SetElementStarShip (FighterElementPtr, StarShipPtr);
		UnlockElement (hFighterElement);
	}
}

static void
turret_missile_preprocess (ELEMENT *ElementPtr)
{		
	// Smart missile tracking system.
	if (ElementPtr->hTarget)
	{
		COUNT facing, num_frames;
		SIZE delta_x, delta_y, delta_facing, missile_speed;
		ELEMENT *TargetPtr;

		facing = ANGLE_TO_FACING (GetVelocityTravelAngle (&ElementPtr->velocity));

		// Transform weapon velocity into cute little world units.
		GetCurrentVelocityComponents (&ElementPtr->velocity, &delta_x, &delta_y);
		missile_speed = VELOCITY_TO_WORLD (square_root (VelocitySquared (delta_x, delta_y)));

		LockElement (ElementPtr->hTarget, &TargetPtr);

		if (TargetPtr->hit_points)
		{
			delta_x = TargetPtr->current.location.x
					- ElementPtr->current.location.x;
			delta_x = WRAP_DELTA_X (delta_x);
			delta_y = TargetPtr->current.location.y
					- ElementPtr->current.location.y;
			delta_y = WRAP_DELTA_Y (delta_y);
	
			num_frames = (square_root ((long)delta_x * delta_x
					+ (long)delta_y * delta_y)) / missile_speed;

			if (num_frames == 0)
				num_frames = 1;
	
			GetNextVelocityComponents (&TargetPtr->velocity,
				&delta_x, &delta_y, num_frames);
	
			delta_x = (TargetPtr->current.location.x + (delta_x / 2))
					- ElementPtr->current.location.x;
			delta_y = (TargetPtr->current.location.y + (delta_y / 2))
					- ElementPtr->current.location.y;
	
			delta_facing = NORMALIZE_FACING (ANGLE_TO_FACING (ARCTAN (delta_x, delta_y)) - facing);

			if (delta_facing > 0 && !OBJECT_CLOAKED(TargetPtr))
			{
				if (delta_facing == ANGLE_TO_FACING (HALF_CIRCLE))
					facing += (((BYTE)TFB_Random () & 1) << 1) - 1;
				else if (delta_facing < ANGLE_TO_FACING (HALF_CIRCLE))
					++facing;
				else
					--facing;

				facing = NORMALIZE_FACING (facing);
			}

			ElementPtr->state_flags |= CHANGING;

			SetVelocityVector (&ElementPtr->velocity, missile_speed, facing);
		}

		UnlockElement (ElementPtr->hTarget);
	}
}

static void
turret_missile_collision (ELEMENT *ElementPtr0, POINT *pPt0, ELEMENT *ElementPtr1, POINT *pPt1)
{
	if (ElementPtr0->pParent != ElementPtr1->pParent)
	{
		//HELEMENT hBlastElement;

		// Use the default weapon damage graphic.
		ElementPtr0->next.image.farray = NULL;
		ElementPtr0->next.image.frame = NULL;
		weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
		ElementPtr0->next.image.farray = ElementPtr0->current.image.farray;
		ElementPtr0->next.image.frame = ElementPtr0->current.image.frame;

		ElementPtr0->state_flags |= COLLISION;
	}
}

static void
initialize_turret (ELEMENT *ElementPtr)
{
	BYTE weakest;
	STARSHIP *StarShipPtr;
	HELEMENT hObject, hNextObject, hBestObject;
	ELEMENT *ShipPtr, *ObjectPtr;
	SIZE delta_x, delta_y;
	long dist, best_dist;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	hBestObject = 0;
	best_dist = TURRET_RANGE + 1;
	weakest = MAX_TARGET_HITS;
	LockElement (StarShipPtr->hShip, &ShipPtr);

	// Is an enemy object within range?
	for (hObject = GetPredElement (ElementPtr); hObject; hObject = hNextObject)
	{
		LockElement (hObject, &ObjectPtr);
		hNextObject = GetPredElement (ObjectPtr);

		if (!elementsOfSamePlayer (ObjectPtr, ShipPtr)
			&& ObjectPtr->playerNr != NEUTRAL_PLAYER_NUM
			&& CollisionPossible (ObjectPtr, ShipPtr)
			&& !OBJECT_CLOAKED (ObjectPtr))
		{
			delta_x = ObjectPtr->next.location.x - ShipPtr->next.location.x;
			delta_y = ObjectPtr->next.location.y - ShipPtr->next.location.y;

			if (delta_x < 0)
				delta_x = -delta_x;
			if (delta_y < 0)
				delta_y = -delta_y;

			// Range check.
			if (delta_x <= TURRET_RANGE
				&& delta_y <= TURRET_RANGE
				&& (dist = (long)delta_x * delta_x + (long)delta_y * delta_y)
				<= (long)TURRET_RANGE * TURRET_RANGE)
			{
				dist = square_root(dist);

				// Lower hitpoints and closer proximity are preferable.
				if (ObjectPtr->hit_points < weakest
					|| (ObjectPtr->hit_points == weakest
					&& dist < best_dist))
				{
					hBestObject = hObject;
					best_dist = dist;
					weakest = ObjectPtr->hit_points;
				}
				// The enemy ship is also a potential target.
				else if (ObjectPtr->state_flags & PLAYER_SHIP
					&& dist < best_dist
					&& hBestObject == 0)
				{
					hBestObject = hObject;
					best_dist = dist;
				}
			}
		}

		UnlockElement (hObject);
	}

	// Fire turret.
	if (hBestObject
		&& DeltaEnergy (ElementPtr, -AUXILIARY_ENERGY_COST))
	{
		HELEMENT hTurretMissile;
		MISSILE_BLOCK MissileBlock;

		LockElement (hBestObject, &ObjectPtr);

		delta_x = WRAP_DELTA_X (ObjectPtr->current.location.x - ShipPtr->current.location.x);
		delta_y = WRAP_DELTA_Y (ObjectPtr->current.location.y - ShipPtr->current.location.y);

		MissileBlock.cx = ShipPtr->next.location.x
			+ COSINE(FACING_TO_ANGLE(StarShipPtr->ShipFacing), -3);
		MissileBlock.cy = ShipPtr->next.location.y
			+ SINE(FACING_TO_ANGLE(StarShipPtr->ShipFacing), -3);
		MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.special;
		MissileBlock.face = NORMALIZE_FACING (ANGLE_TO_FACING (ARCTAN (delta_x, delta_y)));
		MissileBlock.index = 18;
		MissileBlock.sender = ShipPtr->playerNr;
		MissileBlock.flags = IGNORE_SIMILAR;
		MissileBlock.pixoffs = TURRET_OFFSET;
		MissileBlock.speed = TURRET_MISSILE_SPEED;
		MissileBlock.hit_points = TURRET_MISSILE_HITS;
		MissileBlock.damage = TURRET_MISSILE_DAMAGE;
		MissileBlock.life = TURRET_MISSILE_LIFE;
		MissileBlock.preprocess_func = turret_missile_preprocess;
		MissileBlock.blast_offs = TURRET_MISSILE_OFFSET;
		
		hTurretMissile = initialize_missile (&MissileBlock);
		if (hTurretMissile)
		{
			ELEMENT *MissilePtr;

			GetCurrentVelocityComponents (&ShipPtr->velocity, &delta_x, &delta_y);

			LockElement (hTurretMissile, &MissilePtr);

			// Turret missile velocity is relative to the Dreadnought's velocity.
			DeltaVelocityComponents (&MissilePtr->velocity, delta_x, delta_y);
			MissilePtr->current.location.x -= VELOCITY_TO_WORLD (delta_x);
			MissilePtr->current.location.y -= VELOCITY_TO_WORLD (delta_y);

			MissilePtr->hTarget = hBestObject; // Acquire target.
			MissilePtr->collision_func = turret_missile_collision;
			SetElementStarShip (MissilePtr, StarShipPtr);
			UnlockElement (hTurretMissile);

			PutElement (hTurretMissile);
		}

		ProcessSound (SetAbsSoundIndex (
				// Autoturret shoots //
			StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 2), ElementPtr);

		StarShipPtr->auxiliary_counter = AUXILIARY_WAIT;

		UnlockElement (hBestObject);
	}

	UnlockElement (StarShipPtr->hShip);
}

static void
urquan_postprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;
	
	GetElementStarShip	(ElementPtr, &StarShipPtr);

	// Cycle through guns via static_counter after every shot.
	if (StarShipPtr->weapon_counter == 1)
		++StarShipPtr->static_counter;

	// Reset static_counter when necessary.
	if (StarShipPtr->static_counter > 3
			|| StarShipPtr->RaceDescPtr->ship_info.energy_level == MAX_ENERGY
			|| StarShipPtr->RaceDescPtr->ship_info.energy_level < WEAPON_ENERGY_COST)
		StarShipPtr->static_counter = 0;
	
	// Fighter launch code.
	if ((StarShipPtr->cur_status_flags & SPECIAL)
		&& ElementPtr->crew_level > 1
		&& StarShipPtr->special_counter == 0
		&& DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST))
	{
		spawn_fighters (ElementPtr);

		ProcessSound (SetAbsSoundIndex
				/* LAUNCH_FIGHTERS */
			(StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);

		StarShipPtr->special_counter = SPECIAL_WAIT;
	}

	// Autoturret activation code.
	if (StarShipPtr->auxiliary_counter == 0
		&& StarShipPtr->RaceDescPtr->ship_info.energy_level >= AUXILIARY_ENERGY_COST)
	{
		HELEMENT hDefense;

		hDefense = AllocElement ();
		if (hDefense)
		{
			ELEMENT *DefensePtr;
			
			PutElement (hDefense);

			LockElement (hDefense, &DefensePtr);
			DefensePtr->state_flags = APPEARING | NONSOLID | FINITE_LIFE
					| (ElementPtr->state_flags & (GOOD_GUY | BAD_GUY));

			{
				ELEMENT *SuccPtr;

				LockElement (GetSuccElement (ElementPtr), &SuccPtr);
				DefensePtr->hTarget = GetPredElement (SuccPtr);
				UnlockElement (GetSuccElement (ElementPtr));

				DefensePtr->death_func = initialize_turret;
			}

			SetElementStarShip (DefensePtr, StarShipPtr);
			
			UnlockElement (hDefense);
		}
	}
}

static void
urquan_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern,
		COUNT ConcernCounter)
{
	EVALUATE_DESC *lpEvalDesc;
	STARSHIP *StarShipPtr;

	GetElementStarShip (ShipPtr, &StarShipPtr);

	 ObjectsOfConcern[ENEMY_SHIP_INDEX].MoveState = PURSUE;
	lpEvalDesc = &ObjectsOfConcern[ENEMY_WEAPON_INDEX];
	if (lpEvalDesc->ObjectPtr
			&& lpEvalDesc->MoveState == ENTICE
			&& (!(lpEvalDesc->ObjectPtr->state_flags & CREW_OBJECT)
			|| lpEvalDesc->which_turn <= 8)
			&& (!(lpEvalDesc->ObjectPtr->state_flags & FINITE_LIFE)
			|| (lpEvalDesc->ObjectPtr->mass_points >= 4
			&& lpEvalDesc->which_turn == 2
			&& ObjectsOfConcern[ENEMY_SHIP_INDEX].which_turn > 16)))
		lpEvalDesc->MoveState = PURSUE;

	ship_intelligence (ShipPtr,
			ObjectsOfConcern, ConcernCounter);

	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
	{
		STARSHIP *EnemyStarShipPtr = NULL;

		if (lpEvalDesc->ObjectPtr)
			GetElementStarShip (lpEvalDesc->ObjectPtr, &EnemyStarShipPtr);
		if (StarShipPtr->special_counter == 0
				&& lpEvalDesc->ObjectPtr
				&& StarShipPtr->RaceDescPtr->ship_info.crew_level >
				(StarShipPtr->RaceDescPtr->ship_info.max_crew >> 2)
				&& !(EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags
				& HEAVY_POINT_DEFENSE)
				&& (StarShipPtr->RaceDescPtr->characteristics.special_wait < 6
				|| (MANEUVERABILITY (&EnemyStarShipPtr->RaceDescPtr->cyborg_control) <= SLOW_SHIP
					&& !(EnemyStarShipPtr->cur_status_flags & SHIP_BEYOND_MAX_SPEED)
					&& StarShipPtr->RaceDescPtr->ship_info.energy_level >=
						(BYTE)(StarShipPtr->RaceDescPtr->ship_info.max_energy >> 1)
					&& StarShipPtr->RaceDescPtr->characteristics.special_wait < 12)
				|| (lpEvalDesc->which_turn <= 12
					&& (StarShipPtr->ship_input_state & (LEFT | RIGHT))
					&& StarShipPtr->RaceDescPtr->ship_info.energy_level >=
					(BYTE)(StarShipPtr->RaceDescPtr->ship_info.max_energy >> 1))))
			StarShipPtr->ship_input_state |= SPECIAL;
		else
			StarShipPtr->ship_input_state &= ~SPECIAL;
	}

	StarShipPtr->RaceDescPtr->characteristics.special_wait = 0;
}

RACE_DESC*
init_urquan (void)
{
	RACE_DESC *RaceDescPtr;

	urquan_desc.postprocess_func = urquan_postprocess;
	urquan_desc.init_weapon_func = initialize_fusion;
	urquan_desc.cyborg_control.intelligence_func = urquan_intelligence;

	RaceDescPtr = &urquan_desc;

	return (RaceDescPtr);
}

