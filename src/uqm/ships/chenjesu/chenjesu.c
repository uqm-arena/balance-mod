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
#include "chenjesu.h"
#include "resinst.h"
#include "uqm/globdata.h"
#include "libs/mathlib.h"

// Core Characteristics
#define MAX_CREW 36
#define MAX_ENERGY 30
#define ENERGY_REGENERATION 1
#define ENERGY_WAIT 4
#define MAX_THRUST  27
#define THRUST_INCREMENT 3
#define THRUST_WAIT 4
#define TURN_WAIT 6
#define SHIP_MASS 9

// Photon Shard
#define WEAPON_ENERGY_COST 5
#define WEAPON_WAIT 0
#define CHENJESU_OFFSET 16
#define MISSILE_OFFSET 0
#define MISSILE_SPEED 64
#define MISSILE_LIFE 90
#define MISSILE_HITS 10
#define MISSILE_DAMAGE 6
#define NUM_SPARKLES 8

// Shrapnel
#define FRAGMENT_OFFSET 2
#define NUM_FRAGMENTS 8
#define FRAGMENT_SPEED 96
#define FRAGMENT_LIFE 15
#define FRAGMENT_RANGE DISPLAY_TO_WORLD (110) // This bit is for the cyborg only
#define FRAGMENT_HITS 1
#define FRAGMENT_DAMAGE 1

// Shockwave
#define WAVE_LIFE 7
#define WAVE_DAMAGE_DELAY 2
#define WAVE_DAMAGE 1
#define WAVE_OBJ_RANGE 52
#define WAVE_SHIP_RANGE 76
/* Shockwave has longer range against enemy ships because this effect checks its target's central point
   only; the extra range allows this effect to scathe the edge of a ship and still inflict damage */

// DOGI
#define SPECIAL_ENERGY_COST MAX_ENERGY
#define SPECIAL_WAIT 0
#define DOGGY_OFFSET 20
#define DOGGY_SPEED 32
#define ENERGY_DRAIN 10
#define MAX_DOGGIES 4
#define DOGGY_HITS 3
#define DOGGY_MASS 4

static RACE_DESC chenjesu_desc =
{
	{ /* SHIP_INFO */
		FIRES_FORE | SEEKING_SPECIAL | SEEKING_WEAPON,
		25, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		CHENJESU_RACE_STRINGS,
		CHENJESU_ICON_MASK_PMAP_ANIM,
		CHENJESU_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		0, /* Initial sphere of influence radius */
		{ /* Known location (center of SoI) */
			0, 0,
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
			CHENJESU_BIG_MASK_PMAP_ANIM,
			CHENJESU_MED_MASK_PMAP_ANIM,
			CHENJESU_SML_MASK_PMAP_ANIM,
		},
		{
			SPARK_BIG_MASK_PMAP_ANIM,
			SPARK_MED_MASK_PMAP_ANIM,
			SPARK_SML_MASK_PMAP_ANIM,
		},
		{
			DOGGY_BIG_MASK_PMAP_ANIM,
			DOGGY_MED_MASK_PMAP_ANIM,
			DOGGY_SML_MASK_PMAP_ANIM,
		},
		{
			CHENJESU_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		CHENJESU_VICTORY_SONG,
		CHENJESU_SHIP_SOUNDS,
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		NULL, NULL
	},
	{
		0,
		LONG_RANGE_WEAPON,
		NULL,
	},
	(UNINIT_FUNC *) NULL,
	(PREPROCESS_FUNC *) NULL,
	(POSTPROCESS_FUNC *) NULL,
	(INIT_WEAPON_FUNC *) NULL,
	0,
	0, /* CodeRef */
};

// Fragments will not harm friendly DOGIs.
static void
fragment_collision (ELEMENT *ElementPtr0, POINT *pPt0, ELEMENT *ElementPtr1, POINT *pPt1)
{
	STARSHIP *OtherShipPtr;

	GetElementStarShip (ElementPtr1, &OtherShipPtr);
	
	// Ignore DOGIs
	if (OtherShipPtr && OtherShipPtr->SpeciesID == CHENJESU_ID
		&& ElementPtr0->playerNr == ElementPtr1->playerNr
		&& ElementPtr1->mass_points == 4)
	{
		ElementPtr0->mass_points = 0;
	}

	weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
}

void
fragment_preprocess (ELEMENT *ElementPtr)
{
	COUNT facing;
		
	facing = NORMALIZE_FACING (ANGLE_TO_FACING (GetVelocityTravelAngle (&ElementPtr->velocity)));

	if (ElementPtr->thrust_wait == 0)
		// Shrapnel twirls left
		SetVelocityVector (&ElementPtr->velocity, FRAGMENT_SPEED, (facing - 1));
	else
		// Shrapnel twirls right
		SetVelocityVector (&ElementPtr->velocity, FRAGMENT_SPEED, (facing + 1));
}

static void
shockwave_preprocess (ELEMENT *ElementPtr)
{
	ElementPtr->next.image.frame = IncFrameIndex (ElementPtr->current.image.frame);
	ElementPtr->state_flags |= CHANGING;
	
	if (ElementPtr->life_span == WAVE_LIFE - WAVE_DAMAGE_DELAY)
	{
		HELEMENT hElement, hNextElement, hTarget;
		STARSHIP *StarShipPtr, *EnemyStarShipPtr;

		GetElementStarShip (ElementPtr, &StarShipPtr);
		hTarget = 0;

		// Cycle through all objects in the arena
		for (hElement = GetHeadElement ();
			hElement != 0; hElement = hNextElement)
		{
			ELEMENT *ObjPtr;

			LockElement (hElement, &ObjPtr);
			hNextElement = GetSuccElement (ObjPtr);
			if (CollidingElement (ObjPtr))
			{
				SIZE delta_x, delta_y;

				// Begin distance check
				if ((delta_x = ObjPtr->next.location.x
						- ElementPtr->next.location.x) < 0)
					delta_x = -delta_x;
				if ((delta_y = ObjPtr->next.location.y
						- ElementPtr->next.location.y) < 0)
					delta_y = -delta_y;
				delta_x = WORLD_TO_DISPLAY (delta_x);
				delta_y = WORLD_TO_DISPLAY (delta_y);
				
				// Is the enemy ship within range?
				if (ObjPtr->state_flags & PLAYER_SHIP
						&& !elementsOfSamePlayer(ElementPtr, ObjPtr)
						&& delta_x <= WAVE_SHIP_RANGE && delta_y <= WAVE_SHIP_RANGE
						&& ((long)(delta_x * delta_x) + (long)(delta_y * delta_y)) <=
							(long)(WAVE_SHIP_RANGE * WAVE_SHIP_RANGE))
				{
					GetElementStarShip (ObjPtr, &EnemyStarShipPtr);

					// Utwig shield absorbs energy
					if (EnemyStarShipPtr && EnemyStarShipPtr->SpeciesID == UTWIG_ID
							&& ObjPtr->life_span > NORMAL_LIFE)
					{
						ObjPtr->life_span += WAVE_DAMAGE;
					}
					else if (ObjPtr->life_span == NORMAL_LIFE)
					{
						// Damage or destroy enemy ship
						if (!DeltaCrew (ObjPtr, -WAVE_DAMAGE))
							ObjPtr->life_span = 0;
					}
				}
				// Locate non-friendly objects within radius
				else if (!(ObjPtr->state_flags & PLAYER_SHIP)
						&& !elementsOfSamePlayer(ElementPtr, ObjPtr)
						&& !GRAVITY_MASS (ObjPtr->mass_points)
						&& delta_x <= WAVE_OBJ_RANGE && delta_y <= WAVE_OBJ_RANGE
						&& ((long)(delta_x * delta_x) + (long)(delta_y * delta_y)) <=
							(long)(WAVE_OBJ_RANGE * WAVE_OBJ_RANGE))
				{
					// Damage or destroy object
					if (WAVE_DAMAGE < ObjPtr->hit_points)
						ObjPtr->hit_points -= WAVE_DAMAGE;
					else
					{
						ObjPtr->hit_points = 0;
						ObjPtr->life_span = 0;
					}
				}
			}

			UnlockElement (hElement);
		}
	}
}

static void
crystal_postprocess (ELEMENT *ElementPtr)
{
	MISSILE_BLOCK MissileBlock;
	SIZE twirl;
	HELEMENT hWave;
	STARSHIP *StarShipPtr;
    
	// Generate shrapnel
	GetElementStarShip (ElementPtr, &StarShipPtr);
	MissileBlock.cx = ElementPtr->next.location.x;
	MissileBlock.cy = ElementPtr->next.location.y;
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.index = 1;
	MissileBlock.sender = ElementPtr->playerNr;
	MissileBlock.flags = IGNORE_SIMILAR;
	MissileBlock.pixoffs = 0;
	MissileBlock.speed = FRAGMENT_SPEED;
	MissileBlock.hit_points = FRAGMENT_HITS;
	MissileBlock.damage = FRAGMENT_DAMAGE;
	MissileBlock.life = FRAGMENT_LIFE;
	MissileBlock.preprocess_func = fragment_preprocess;
	MissileBlock.blast_offs = FRAGMENT_OFFSET;
    twirl = (BYTE)TFB_Random () & 1;
    
	for (MissileBlock.face = 0;
			MissileBlock.face < ANGLE_TO_FACING (FULL_CIRCLE);
			MissileBlock.face +=
			(ANGLE_TO_FACING (FULL_CIRCLE) / NUM_FRAGMENTS))
	{
		HELEMENT hFragment;

		hFragment = initialize_missile (&MissileBlock);
		if (hFragment)
		{
			ELEMENT *FragPtr;

			LockElement (hFragment, &FragPtr);
			SetElementStarShip (FragPtr, StarShipPtr);
            FragPtr->collision_func = fragment_collision;
            
			if (twirl == 0) // Left or right?
				FragPtr->thrust_wait = 0;
			else
				FragPtr->thrust_wait = 1;
            
			UnlockElement (hFragment);
			PutElement (hFragment);
		}
	}

	// Generate shockwave
	hWave = AllocElement ();
	if (hWave)
	{
		ELEMENT *WavePtr;

		LockElement (hWave, &WavePtr);
		WavePtr->current.location = ElementPtr->next.location;
		WavePtr->life_span = WAVE_LIFE;
		WavePtr->current.image.farray = StarShipPtr->RaceDescPtr->ship_data.special;
		WavePtr->current.image.frame =
			SetAbsFrameIndex (WavePtr->current.image.farray[0], 7);
		WavePtr->preprocess_func = shockwave_preprocess;
		WavePtr->playerNr = ElementPtr->playerNr;
		WavePtr->state_flags = APPEARING | NONSOLID | FINITE_LIFE;
		SetPrimType (&(GLOBAL (DisplayArray))[WavePtr->PrimIndex], STAMP_PRIM);
		SetElementStarShip (WavePtr, StarShipPtr);
		UnlockElement (hWave);
		PutElement (hWave);
	}

	ProcessSound (SetAbsSoundIndex (
					/* CRYSTAL_FRAGMENTS */
			StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), ElementPtr);
}

static void
crystal_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	if (StarShipPtr->cur_status_flags & WEAPON)
		++ElementPtr->life_span; /* keep it going while key pressed */
	else
	{
		ElementPtr->life_span = 1;

		ElementPtr->postprocess_func = crystal_postprocess;
	}
}

static void
animate (ELEMENT *ElementPtr)
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
crystal_collision (ELEMENT *ElementPtr0, POINT *pPt0,
		ELEMENT *ElementPtr1, POINT *pPt1)
{
	HELEMENT hBlastElement;

	hBlastElement =
			weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
	if (hBlastElement)
	{
		ELEMENT *BlastElementPtr;
        
		LockElement (hBlastElement, &BlastElementPtr);
		BlastElementPtr->current.location = ElementPtr1->current.location;

		BlastElementPtr->life_span = NUM_SPARKLES;
		BlastElementPtr->turn_wait = BlastElementPtr->next_turn = 0;
		{
			BlastElementPtr->preprocess_func = animate;
		}

		BlastElementPtr->current.image.farray = ElementPtr0->next.image.farray;
		BlastElementPtr->current.image.frame =
				SetAbsFrameIndex (BlastElementPtr->current.image.farray[0],
				2); /* skip stones */

		UnlockElement (hBlastElement);
	}
}

static void
doggy_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	++StarShipPtr->special_counter;
	if (ElementPtr->thrust_wait > 0) /* could be non-zero after a collision */
		--ElementPtr->thrust_wait;
	else
	{
		COUNT facing, orig_facing;
		SIZE delta_facing;

		facing = orig_facing =
				NORMALIZE_FACING (ANGLE_TO_FACING (
				GetVelocityTravelAngle (&ElementPtr->velocity)
				));
		if ((delta_facing = TrackShip (ElementPtr, &facing)) < 0)
			facing = NORMALIZE_FACING (TFB_Random ());
		else
		{
			ELEMENT *ShipPtr;

			LockElement (ElementPtr->hTarget, &ShipPtr);
			facing = NORMALIZE_FACING (ANGLE_TO_FACING (
					ARCTAN (ShipPtr->current.location.x -
					ElementPtr->current.location.x,
					ShipPtr->current.location.y -
					ElementPtr->current.location.y)
					));
			delta_facing = NORMALIZE_FACING (facing -
					GetFrameIndex (ShipPtr->current.image.frame));
			UnlockElement (ElementPtr->hTarget);

			if (delta_facing > ANGLE_TO_FACING (HALF_CIRCLE - OCTANT) &&
					delta_facing < ANGLE_TO_FACING (HALF_CIRCLE + OCTANT))
			{
				if (delta_facing >= ANGLE_TO_FACING (HALF_CIRCLE))
					facing -= ANGLE_TO_FACING (QUADRANT);
				else
					facing += ANGLE_TO_FACING (QUADRANT);
			}

			facing = NORMALIZE_FACING (facing);
		}

		if (facing != orig_facing)
			SetVelocityVector (&ElementPtr->velocity, DOGGY_SPEED, facing);
	}
}

static void
doggy_death (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	ProcessSound (SetAbsSoundIndex (
					/* DOGGY_DIES */
			StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 3), ElementPtr);

	ElementPtr->state_flags &= ~DISAPPEARING;
	ElementPtr->state_flags |= NONSOLID | FINITE_LIFE;
	ElementPtr->life_span = 6;
	{
		ElementPtr->preprocess_func = animate;
	}
	ElementPtr->death_func = NULL;
	ElementPtr->collision_func = NULL;
	ZeroVelocityComponents (&ElementPtr->velocity);

	ElementPtr->turn_wait = ElementPtr->next_turn = 0;
}

static void
doggy_collision (ELEMENT *ElementPtr0, POINT *pPt0,
		ELEMENT *ElementPtr1, POINT *pPt1)
{
	collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
	if ((ElementPtr1->state_flags & PLAYER_SHIP)
			&& !elementsOfSamePlayer (ElementPtr0, ElementPtr1))
	{
		STARSHIP *StarShipPtr;

		GetElementStarShip (ElementPtr0, &StarShipPtr);
		ProcessSound (SetAbsSoundIndex (
						/* DOGGY_STEALS_ENERGY */
				StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 2), ElementPtr0);
		GetElementStarShip (ElementPtr1, &StarShipPtr);
		if (StarShipPtr->RaceDescPtr->ship_info.energy_level < ENERGY_DRAIN)
			DeltaEnergy (ElementPtr1, -StarShipPtr->RaceDescPtr->ship_info.energy_level);
		else
			DeltaEnergy (ElementPtr1, -ENERGY_DRAIN);
	}
	if (ElementPtr0->thrust_wait <= COLLISION_THRUST_WAIT)
		ElementPtr0->thrust_wait += COLLISION_THRUST_WAIT << 1;
}

static void
spawn_doggy (ELEMENT *ElementPtr)
{
	HELEMENT hDoggyElement;
	STARSHIP *StarShipPtr;
    
	GetElementStarShip (ElementPtr, &StarShipPtr);
    
	if (StarShipPtr->hShip)
	{
		LockElement (StarShipPtr->hShip, &ElementPtr);
    
		if ((hDoggyElement = AllocElement ()) != 0)
		{
			COUNT angle;
			ELEMENT *DoggyElementPtr;

			ElementPtr->state_flags |= DEFY_PHYSICS;

			PutElement (hDoggyElement);
			LockElement (hDoggyElement, &DoggyElementPtr);
			DoggyElementPtr->hit_points = DOGGY_HITS;
			DoggyElementPtr->mass_points = DOGGY_MASS;
			DoggyElementPtr->thrust_wait = 0;
			DoggyElementPtr->playerNr = ElementPtr->playerNr;
			DoggyElementPtr->state_flags = APPEARING;
			DoggyElementPtr->life_span = NORMAL_LIFE;

			DoggyElementPtr->triggers_teleport_safety = TRUE;
    
			SetPrimType (&(GLOBAL (DisplayArray))[DoggyElementPtr->PrimIndex],
				     STAMP_PRIM);
			{
				DoggyElementPtr->preprocess_func = doggy_preprocess;
				DoggyElementPtr->postprocess_func = NULL;
				DoggyElementPtr->collision_func = doggy_collision;
				DoggyElementPtr->death_func = doggy_death;
			}

			angle = FACING_TO_ANGLE (StarShipPtr->ShipFacing) + HALF_CIRCLE;
			DoggyElementPtr->current.location.x = ElementPtr->next.location.x
					+ COSINE (angle, DISPLAY_TO_WORLD (CHENJESU_OFFSET + DOGGY_OFFSET));
			DoggyElementPtr->current.location.y = ElementPtr->next.location.y
					+ SINE (angle, DISPLAY_TO_WORLD (CHENJESU_OFFSET + DOGGY_OFFSET));
			DoggyElementPtr->current.image.farray = StarShipPtr->RaceDescPtr->ship_data.special;
			DoggyElementPtr->current.image.frame = StarShipPtr->RaceDescPtr->ship_data.special[0];

			SetVelocityVector (&DoggyElementPtr->velocity,
					DOGGY_SPEED, NORMALIZE_FACING (ANGLE_TO_FACING (angle)));

			SetElementStarShip (DoggyElementPtr, StarShipPtr);
			ProcessSound (SetAbsSoundIndex (
						/* RELEASE_DOGGY */
				StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 4), DoggyElementPtr);
			UnlockElement (hDoggyElement);
	}
       
        UnlockElement (StarShipPtr->hShip);
	}
}

static COUNT
initialize_crystal (ELEMENT *ShipPtr, HELEMENT CrystalArray[])
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
	MissileBlock.pixoffs = CHENJESU_OFFSET;
	MissileBlock.speed = MISSILE_SPEED;
	MissileBlock.hit_points = MISSILE_HITS;
	MissileBlock.damage = MISSILE_DAMAGE;
	MissileBlock.life = MISSILE_LIFE;
	MissileBlock.preprocess_func = crystal_preprocess;
	MissileBlock.blast_offs = MISSILE_OFFSET;
	CrystalArray[0] = initialize_missile (&MissileBlock);

	if (CrystalArray[0])
	{
        SIZE dx, dy;
		ELEMENT *CrystalPtr;
        
        LockElement (CrystalArray[0], &CrystalPtr);
        
		GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
		dx = dx * 1/2;
		dy = dy * 1/2;
        
		// Add some of the Broodhome's velocity to its projectiles
		DeltaVelocityComponents (&CrystalPtr->velocity, dx, dy);
		CrystalPtr->current.location.x -= VELOCITY_TO_WORLD (dx);
		CrystalPtr->current.location.y -= VELOCITY_TO_WORLD (dy);
        
		CrystalPtr->collision_func = crystal_collision;
		UnlockElement (CrystalArray[0]);
	}

	return (1);
}

static void
chenjesu_postprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	if ((StarShipPtr->cur_status_flags & SPECIAL)
			&& StarShipPtr->special_counter < MAX_DOGGIES
			&& DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST))
	{
		spawn_doggy (ElementPtr);
	}

	StarShipPtr->special_counter = 1;
	// say there is one doggy, because ship_postprocess will decrement special_counter
}

static void
chenjesu_preprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);

	if (StarShipPtr->special_counter > 1) // only when STANDARD computer opponent (??)
		StarShipPtr->special_counter += MAX_DOGGIES;

	// Prevent more than one shot while holding down 'Fire'
	if (StarShipPtr->cur_status_flags
			& StarShipPtr->old_status_flags & WEAPON)
		++StarShipPtr->weapon_counter;
}

static void
chenjesu_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern,
		COUNT ConcernCounter)
{
	EVALUATE_DESC *lpEvalDesc;
	STARSHIP *StarShipPtr, *EnemyStarShipPtr;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	StarShipPtr->ship_input_state &= ~SPECIAL;

	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
	if (lpEvalDesc->ObjectPtr)
	{
		GetElementStarShip (lpEvalDesc->ObjectPtr, &EnemyStarShipPtr);
		if ((lpEvalDesc->which_turn <= 16
				&& MANEUVERABILITY (&EnemyStarShipPtr->RaceDescPtr->cyborg_control) >= MEDIUM_SHIP)
				|| (MANEUVERABILITY (&EnemyStarShipPtr->RaceDescPtr->cyborg_control) <= SLOW_SHIP
					&& WEAPON_RANGE (&EnemyStarShipPtr->RaceDescPtr->cyborg_control) >= LONG_RANGE_WEAPON * 3 / 4
					&& (EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags & SEEKING_WEAPON))
				|| (EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags & HEAVY_POINT_DEFENSE))
			lpEvalDesc->MoveState = PURSUE;
	}

	if (StarShipPtr->special_counter == 1
			&& ObjectsOfConcern[ENEMY_WEAPON_INDEX].ObjectPtr
			&& ObjectsOfConcern[ENEMY_WEAPON_INDEX].MoveState == ENTICE
			&& ObjectsOfConcern[ENEMY_WEAPON_INDEX].which_turn <= 8)
	{
		lpEvalDesc = &ObjectsOfConcern[ENEMY_WEAPON_INDEX];
	}

	ship_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter);

	if (lpEvalDesc->ObjectPtr)
	{
		HELEMENT h, hNext;
		ELEMENT *CrystalPtr;

		h = (StarShipPtr->old_status_flags & WEAPON) ?
				GetTailElement () : (HELEMENT)0;
		for (; h; h = hNext)
		{
			LockElement (h, &CrystalPtr);
			hNext = GetPredElement (CrystalPtr);
			if (!(CrystalPtr->state_flags & NONSOLID)
					&& CrystalPtr->next.image.farray == StarShipPtr->RaceDescPtr->ship_data.weapon
					&& CrystalPtr->preprocess_func
					&& CrystalPtr->life_span > 0
					&& elementsOfSamePlayer (CrystalPtr, ShipPtr))
			{
				if (ObjectsOfConcern[ENEMY_SHIP_INDEX].ObjectPtr)
				{
					COUNT which_turn;

					if ((which_turn = PlotIntercept (CrystalPtr,
							ObjectsOfConcern[ENEMY_SHIP_INDEX].ObjectPtr,
							CrystalPtr->life_span,
							FRAGMENT_RANGE / 2)) == 0
							|| (which_turn == 1
							&& PlotIntercept (CrystalPtr,
							ObjectsOfConcern[ENEMY_SHIP_INDEX].ObjectPtr,
							CrystalPtr->life_span, 0) == 0))
						StarShipPtr->ship_input_state &= ~WEAPON;
					else if (StarShipPtr->weapon_counter == 0)
					{
						StarShipPtr->ship_input_state |= WEAPON;
						lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
					}

					UnlockElement (h);
					break;
				}
				hNext = 0;
			}
			UnlockElement (h);
		}

		if (h == 0)
		{
			if (StarShipPtr->old_status_flags & WEAPON)
			{
				StarShipPtr->ship_input_state &= ~WEAPON;
				if (lpEvalDesc == &ObjectsOfConcern[ENEMY_WEAPON_INDEX])
					StarShipPtr->weapon_counter = 3;
			}
			else if (StarShipPtr->weapon_counter == 0
					&& ship_weapons (ShipPtr, lpEvalDesc->ObjectPtr, FRAGMENT_RANGE / 2))
				StarShipPtr->ship_input_state |= WEAPON;
		}
	}

	if (StarShipPtr->special_counter < MAX_DOGGIES)
	{
		if (lpEvalDesc->ObjectPtr
				&& StarShipPtr->RaceDescPtr->ship_info.energy_level > SPECIAL_ENERGY_COST >> 1
				&& !(StarShipPtr->ship_input_state & WEAPON)
				&& !(EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags & HEAVY_POINT_DEFENSE))
			StarShipPtr->ship_input_state |= SPECIAL;
	}
}

RACE_DESC*
init_chenjesu (void)
{
	RACE_DESC *RaceDescPtr;

	chenjesu_desc.preprocess_func = chenjesu_preprocess;
	chenjesu_desc.postprocess_func = chenjesu_postprocess;
	chenjesu_desc.init_weapon_func = initialize_crystal;
	chenjesu_desc.cyborg_control.intelligence_func = chenjesu_intelligence;

	RaceDescPtr = &chenjesu_desc;

	return (RaceDescPtr);
}

