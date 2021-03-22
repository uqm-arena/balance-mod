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
#include "melnorme.h"
#include "resinst.h"
#include "uqm/colors.h"
#include "uqm/globdata.h"
#include "uqm/setup.h"
#include "libs/mathlib.h"

// Core Characteristics
#define MAX_CREW 20
#define MAX_ENERGY MAX_ENERGY_SIZE
#define ENERGY_REGENERATION 1
#define ENERGY_WAIT 4
#define MAX_THRUST 36
#define THRUST_INCREMENT 6
#define THRUST_WAIT 4
#define TURN_WAIT 4
#define SHIP_MASS 7

// Blaster Pulse
#define WEAPON_ENERGY_COST 5
#define WEAPON_WAIT 1
#define MELNORME_OFFSET 24
#define LEVEL_COUNTER 72
#define MAX_PUMP 4
#define PUMPUP_SPEED 180
#define PUMPUP_LIFE 10
#define PUMPUP_DAMAGE 2
#define MIN_PUMPITUDE_ANIMS 3
#define NUM_PUMP_ANIMS 5
#define REVERSE_DIR (BYTE)(1 << 7)

// Confusion Pulse
#define SPECIAL_ENERGY_COST 20
#define SPECIAL_WAIT 20
#define CMISSILE_SPEED 120
#define CMISSILE_LIFE 20
#define CMISSILE_HITS 50
#define CMISSILE_DAMAGE 0
#define CMISSILE_OFFSET 4

// Holographic Targeting Aid
#define HOLO_WAIT 12

static RACE_DESC melnorme_desc =
{
	{ /* SHIP_INFO */
		FIRES_FORE,
		20, /* Super Melee cost */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		MELNORME_RACE_STRINGS,
		MELNORME_ICON_MASK_PMAP_ANIM,
		MELNORME_MICON_MASK_PMAP_ANIM,
		NULL, NULL, NULL
	},
	{ /* FLEET_STUFF */
		INFINITE_RADIUS, /* Initial sphere of influence radius */
		{ /* Known location (center of SoI) */
			MAX_X_UNIVERSE >> 1, MAX_Y_UNIVERSE >> 1,
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
			MELNORME_BIG_MASK_PMAP_ANIM,
			MELNORME_MED_MASK_PMAP_ANIM,
			MELNORME_SML_MASK_PMAP_ANIM,
		},
		{
			PUMPUP_BIG_MASK_PMAP_ANIM,
			PUMPUP_MED_MASK_PMAP_ANIM,
			PUMPUP_SML_MASK_PMAP_ANIM,
		},
		{
			CONFUSE_BIG_MASK_PMAP_ANIM,
			CONFUSE_MED_MASK_PMAP_ANIM,
			CONFUSE_SML_MASK_PMAP_ANIM,
		},
		{
			MELNORME_CAPTAIN_MASK_PMAP_ANIM,
			NULL, NULL, NULL, NULL, NULL
		},
		MELNORME_VICTORY_SONG,
		MELNORME_SHIP_SOUNDS,
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		{ NULL, NULL, NULL },
		NULL, NULL
	},
	{
		0,
		PUMPUP_SPEED * PUMPUP_LIFE,
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
pump_up_preprocess (ELEMENT *ElementPtr)
{
	if (--ElementPtr->thrust_wait & 1)
	{
		COUNT frame_index;

		frame_index = GetFrameIndex (ElementPtr->current.image.frame);
		if (((ElementPtr->turn_wait & REVERSE_DIR)
				&& (frame_index % NUM_PUMP_ANIMS) != 0)
				|| (!(ElementPtr->turn_wait & REVERSE_DIR)
				&& ((frame_index + 1) % NUM_PUMP_ANIMS) == 0))
		{
			--frame_index;
			ElementPtr->turn_wait |= REVERSE_DIR;
		}
		else
		{
			++frame_index;
			ElementPtr->turn_wait &= ~REVERSE_DIR;
		}

		ElementPtr->next.image.frame = SetAbsFrameIndex (
				ElementPtr->current.image.frame, frame_index);

		ElementPtr->state_flags |= CHANGING;
	}
}

static COUNT initialize_pump_up (ELEMENT *ShipPtr, HELEMENT PumpUpArray[]);

static void
pump_up_postprocess (ELEMENT *ElementPtr)
{
	if (ElementPtr->state_flags & APPEARING)
	{
		ZeroVelocityComponents (&ElementPtr->velocity);
	}
	else
	{
		HELEMENT hPumpUp;
		ELEMENT *EPtr;
		ELEMENT *ShipPtr;
		STARSHIP *StarShipPtr;

		GetElementStarShip (ElementPtr, &StarShipPtr);
		LockElement (StarShipPtr->hShip, &ShipPtr);
		initialize_pump_up (ShipPtr, &hPumpUp);
		DeltaEnergy (ShipPtr, 0);
		UnlockElement (StarShipPtr->hShip);

		LockElement (hPumpUp, &EPtr);

		EPtr->current.image.frame = ElementPtr->current.image.frame;
		EPtr->turn_wait = ElementPtr->turn_wait;
		EPtr->thrust_wait = ElementPtr->thrust_wait;
		if (--EPtr->thrust_wait == 0)
		{
			if ((EPtr->turn_wait & ~REVERSE_DIR) < MAX_PUMP - 1)
			{
				++EPtr->turn_wait;
				EPtr->current.image.frame = SetRelFrameIndex (
						EPtr->current.image.frame, NUM_PUMP_ANIMS);
				ProcessSound (SetAbsSoundIndex (
						StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 2),
						EPtr);
			}
			EPtr->thrust_wait = LEVEL_COUNTER;
		}

		EPtr->mass_points = EPtr->hit_points =
				(PUMPUP_DAMAGE << (ElementPtr->turn_wait & ~REVERSE_DIR));
		SetElementStarShip (EPtr, StarShipPtr);

		if (EPtr->thrust_wait & 1)
		{
			COUNT frame_index;

			frame_index = GetFrameIndex (EPtr->current.image.frame);
			if (((EPtr->turn_wait & REVERSE_DIR)
					&& (frame_index % NUM_PUMP_ANIMS) != 0)
					|| (!(EPtr->turn_wait & REVERSE_DIR)
					&& ((frame_index + 1) % NUM_PUMP_ANIMS) == 0))
			{
				--frame_index;
				EPtr->turn_wait |= REVERSE_DIR;
			}
			else
			{
				++frame_index;
				EPtr->turn_wait &= ~REVERSE_DIR;
			}

			EPtr->current.image.frame = SetAbsFrameIndex (
					EPtr->current.image.frame, frame_index);
		}

		if (StarShipPtr->cur_status_flags & StarShipPtr->old_status_flags
				& WEAPON)
		{
			StarShipPtr->weapon_counter = WEAPON_WAIT;
		}
		else
		{
			SIZE dx, dy;
			COUNT angle;

			EPtr->life_span = PUMPUP_LIFE;
			EPtr->preprocess_func = pump_up_preprocess;
			EPtr->postprocess_func = 0;

			angle = FACING_TO_ANGLE (StarShipPtr->ShipFacing);
			SetVelocityComponents (&EPtr->velocity,
					COSINE (angle, WORLD_TO_VELOCITY (PUMPUP_SPEED)),
					SINE (angle, WORLD_TO_VELOCITY (PUMPUP_SPEED)));
			GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
			dx = dx * 3/4;
			dy = dy * 3/4;

			// Add some of the Trader's velocity to its projectiles
			DeltaVelocityComponents (&EPtr->velocity, dx, dy);
			EPtr->current.location.x -= VELOCITY_TO_WORLD (dx);
			EPtr->current.location.y -= VELOCITY_TO_WORLD (dy);

			ProcessSound (SetAbsSoundIndex (
					StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 3), EPtr);
		}

		UnlockElement (hPumpUp);
		PutElement (hPumpUp);

		SetPrimType (&(GLOBAL (DisplayArray))[ElementPtr->PrimIndex],
				NO_PRIM);
		ElementPtr->state_flags |= NONSOLID;
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
pump_up_collision (ELEMENT *ElementPtr0, POINT *pPt0,
		ELEMENT *ElementPtr1, POINT *pPt1)
{
	RECT r;
	BYTE old_thrust_wait;
	HELEMENT hBlastElement;

	GetFrameRect (ElementPtr0->next.image.frame, &r);

	old_thrust_wait = ElementPtr0->thrust_wait;
	ElementPtr0->blast_offset = r.extent.width >> 1;
	hBlastElement = weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);

	// This new section kills suspended blaster pulses after sustaining massive burst damage
	if (ElementPtr1->mass_points >= ElementPtr0->mass_points)
	{
		ElementPtr0->postprocess_func = 0;
		ElementPtr0->thrust_wait = 0;
		ElementPtr0->state_flags |= DISAPPEARING;
	}
	else
		ElementPtr0->thrust_wait = old_thrust_wait;

	if (hBlastElement)
	{
		ELEMENT *BlastElementPtr;

		LockElement (hBlastElement, &BlastElementPtr);

		BlastElementPtr->life_span =
				MIN_PUMPITUDE_ANIMS
				+ (ElementPtr0->turn_wait & ~REVERSE_DIR);
		BlastElementPtr->turn_wait = BlastElementPtr->next_turn = 0;
		{
			BlastElementPtr->preprocess_func = animate;
		}

		BlastElementPtr->current.image.farray = ElementPtr0->next.image.farray;
		BlastElementPtr->current.image.frame =
				SetAbsFrameIndex (BlastElementPtr->current.image.farray[0],
				MAX_PUMP * NUM_PUMP_ANIMS);

		UnlockElement (hBlastElement);
	}
}

static COUNT
initialize_pump_up (ELEMENT *ShipPtr, HELEMENT PumpUpArray[])
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
	MissileBlock.pixoffs = MELNORME_OFFSET;
	MissileBlock.speed = DISPLAY_TO_WORLD (MELNORME_OFFSET);
	MissileBlock.hit_points = PUMPUP_DAMAGE;
	MissileBlock.damage = PUMPUP_DAMAGE;
	MissileBlock.life = 2;
	MissileBlock.preprocess_func = 0;
	MissileBlock.blast_offs = 0;
	PumpUpArray[0] = initialize_missile (&MissileBlock);

	if (PumpUpArray[0])
	{
		ELEMENT *PumpUpPtr;

		LockElement (PumpUpArray[0], &PumpUpPtr);
		PumpUpPtr->postprocess_func = pump_up_postprocess;
		PumpUpPtr->collision_func = pump_up_collision;
		PumpUpPtr->thrust_wait = LEVEL_COUNTER;
		UnlockElement (PumpUpArray[0]);
	}

	return (1);
}

static void
confuse_preprocess (ELEMENT *ElementPtr)
{
	if (!(ElementPtr->state_flags & NONSOLID))
	{
		ElementPtr->next.image.frame = SetAbsFrameIndex (
				ElementPtr->current.image.frame,
				(GetFrameIndex (ElementPtr->current.image.frame) + 1) & 7);
		ElementPtr->state_flags |= CHANGING;
	}
	else if (ElementPtr->hTarget == 0)
	{
		ElementPtr->life_span = 0;
		ElementPtr->state_flags |= DISAPPEARING;
	}
	else
	{
		ELEMENT *eptr;

		LockElement (ElementPtr->hTarget, &eptr);

		ElementPtr->next.location = eptr->next.location;

		if (ElementPtr->turn_wait)
		{
			HELEMENT hEffect;
			STARSHIP *StarShipPtr;

			if (GetFrameIndex (ElementPtr->next.image.frame =
					IncFrameIndex (ElementPtr->current.image.frame)) == 0)
				ElementPtr->next.image.frame =
						SetRelFrameIndex (ElementPtr->next.image.frame, -8);

			GetElementStarShip (eptr, &StarShipPtr);
			StarShipPtr->ship_input_state =
					(StarShipPtr->ship_input_state
					& ~(LEFT | RIGHT | DOWN | SPECIAL))
					| ElementPtr->turn_wait;

			hEffect = AllocElement ();
			if (hEffect)
			{
				LockElement (hEffect, &eptr);
				eptr->playerNr = ElementPtr->playerNr;
				eptr->state_flags = FINITE_LIFE | NONSOLID | CHANGING;
				eptr->life_span = 1;
				eptr->current = eptr->next = ElementPtr->next;
				eptr->preprocess_func = confuse_preprocess;
				SetPrimType (&(GLOBAL (DisplayArray))[eptr->PrimIndex],
						STAMP_PRIM);

				GetElementStarShip (ElementPtr, &StarShipPtr);
				SetElementStarShip (eptr, StarShipPtr);
				eptr->hTarget = ElementPtr->hTarget;

				UnlockElement (hEffect);
				PutElement (hEffect);
			}
		}

		UnlockElement (ElementPtr->hTarget);
	}
}

static void
confusion_collision (ELEMENT *ElementPtr0, POINT *pPt0,
		ELEMENT *ElementPtr1, POINT *pPt1)
{
	if (ElementPtr1->state_flags & PLAYER_SHIP)
	{
		HELEMENT hConfusionElement, hNextElement;
		ELEMENT *ConfusionPtr;
		STARSHIP *StarShipPtr;

		GetElementStarShip (ElementPtr0, &StarShipPtr);
		for (hConfusionElement = GetHeadElement ();
				hConfusionElement; hConfusionElement = hNextElement)
		{
			LockElement (hConfusionElement, &ConfusionPtr);
			if (elementsOfSamePlayer (ConfusionPtr, ElementPtr0)
					&& ConfusionPtr->current.image.farray ==
					StarShipPtr->RaceDescPtr->ship_data.special
					&& (ConfusionPtr->state_flags & NONSOLID))
			{
				UnlockElement (hConfusionElement);
				break;
			}
			hNextElement = GetSuccElement (ConfusionPtr);
			UnlockElement (hConfusionElement);
		}

		if (hConfusionElement || (hConfusionElement = AllocElement ()))
		{
			LockElement (hConfusionElement, &ConfusionPtr);

			if (ConfusionPtr->state_flags == 0) /* not allocated before */
			{
				InsertElement (hConfusionElement, GetHeadElement ());

				ConfusionPtr->current = ElementPtr0->next;
				ConfusionPtr->current.image.frame = SetAbsFrameIndex (
						ConfusionPtr->current.image.frame, 8
						);
				ConfusionPtr->next = ConfusionPtr->current;
				ConfusionPtr->playerNr = ElementPtr0->playerNr;
				ConfusionPtr->state_flags = FINITE_LIFE | NONSOLID | CHANGING;
				ConfusionPtr->preprocess_func = confuse_preprocess;
				SetPrimType (
						&(GLOBAL (DisplayArray))[ConfusionPtr->PrimIndex],
						NO_PRIM
						);

				SetElementStarShip (ConfusionPtr, StarShipPtr);
				GetElementStarShip (ElementPtr1, &StarShipPtr);
				ConfusionPtr->hTarget = StarShipPtr->hShip;
			}

			ConfusionPtr->life_span = 400;
			ConfusionPtr->turn_wait =
					(BYTE)(1 << ((BYTE)TFB_Random () & 1)); /* LEFT or RIGHT */

			UnlockElement (hConfusionElement);
		}

		ElementPtr0->hit_points = 0;
		ElementPtr0->life_span = 0;
		ElementPtr0->state_flags |= DISAPPEARING | COLLISION | NONSOLID;
	}
	(void) pPt0;  /* Satisfying compiler (unused parameter) */
	(void) pPt1;  /* Satisfying compiler (unused parameter) */
}

static COUNT
initialize_confusion (ELEMENT *ShipPtr, HELEMENT ConfusionArray[])
{
	STARSHIP *StarShipPtr;
	MISSILE_BLOCK ConfusionBlock;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	ConfusionBlock.cx = ShipPtr->next.location.x;
	ConfusionBlock.cy = ShipPtr->next.location.y;
	ConfusionBlock.farray = StarShipPtr->RaceDescPtr->ship_data.special;
	ConfusionBlock.index = 0;
	ConfusionBlock.face = StarShipPtr->ShipFacing;
	ConfusionBlock.sender = ShipPtr->playerNr;
	ConfusionBlock.flags = IGNORE_SIMILAR;
	ConfusionBlock.pixoffs = MELNORME_OFFSET;
	ConfusionBlock.speed = CMISSILE_SPEED;
	ConfusionBlock.hit_points = CMISSILE_HITS;
	ConfusionBlock.damage = CMISSILE_DAMAGE;
	ConfusionBlock.life = CMISSILE_LIFE;
	ConfusionBlock.preprocess_func = confuse_preprocess;
	ConfusionBlock.blast_offs = CMISSILE_OFFSET;
	ConfusionArray[0] = initialize_missile (&ConfusionBlock);

	if (ConfusionArray[0])
	{
		ELEMENT *CMissilePtr;
		SIZE dx,dy;

		LockElement (ConfusionArray[0], &CMissilePtr);
		CMissilePtr->collision_func = confusion_collision;
		SetElementStarShip (CMissilePtr, StarShipPtr);
		
		GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
		dx = dx * 3/4;
		dy = dy * 3/4;

		// Add some of the Trader's velocity to its projectiles
		DeltaVelocityComponents (&CMissilePtr->velocity, dx, dy);
		CMissilePtr->current.location.x -= VELOCITY_TO_WORLD (dx);
		CMissilePtr->current.location.y -= VELOCITY_TO_WORLD (dy);
		
		UnlockElement (ConfusionArray[0]);
	}
	return (1);
}

// Trail effect for aim-assist function
static void
spawn_holo_trail (ELEMENT *ElementPtr)
{
	PRIMITIVE *lpPrim;
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);

	// Adjust holo trail position to match starship's drift
	if (StarShipPtr->hShip)
	{
		SIZE dx, dy;
		ELEMENT *ShipPtr;

		LockElement (StarShipPtr->hShip, &ShipPtr);

		GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
		ElementPtr->current.location.x += VELOCITY_TO_WORLD (dx);
		ElementPtr->current.location.y += VELOCITY_TO_WORLD (dy);

		UnlockElement (StarShipPtr->hShip);
	}

	if (ElementPtr->hit_points == 200) // Main projectile
	{
		HELEMENT hTrailElement;

		hTrailElement = AllocElement ();
		if (hTrailElement)
		{
			ELEMENT *TrailElementPtr;
			
			InsertElement (hTrailElement, GetHeadElement ());
			LockElement (hTrailElement, &TrailElementPtr);
			lpPrim = &(GLOBAL (DisplayArray))[TrailElementPtr->PrimIndex];
			TrailElementPtr->state_flags = APPEARING | FINITE_LIFE | NONSOLID | BACKGROUND_OBJECT;
			TrailElementPtr->life_span = TrailElementPtr->thrust_wait = 1;
			TrailElementPtr->current.image.frame = IncFrameIndex (ElementPtr->current.image.frame);
			TrailElementPtr->current.image.farray = ElementPtr->current.image.farray;
			TrailElementPtr->current.location = ElementPtr->current.location;
			TrailElementPtr->death_func = spawn_holo_trail;
			TrailElementPtr->turn_wait = 0;
			SetPrimType (&(GLOBAL (DisplayArray))[TrailElementPtr->PrimIndex], STAMP_PRIM);
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
	else // After-image
	{		
		if (ElementPtr->turn_wait < 1)
		{
			lpPrim = &(GLOBAL (DisplayArray))[ElementPtr->PrimIndex];

			ElementPtr->life_span = ElementPtr->thrust_wait;
			// Reset the life span

			++ElementPtr->turn_wait;
			
			ElementPtr->next.image.frame = IncFrameIndex (ElementPtr->current.image.frame);
					
			ElementPtr->state_flags &= ~DISAPPEARING;
			ElementPtr->state_flags |= CHANGING;
		}
	}
}

// Launch aim-assist projectile
static COUNT
initialize_tracer (ELEMENT *ShipPtr, HELEMENT TracerArray[])
{
	STARSHIP *StarShipPtr;
	MISSILE_BLOCK MissileBlock;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	MissileBlock.cx = ShipPtr->next.location.x;
	MissileBlock.cy = ShipPtr->next.location.y;
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.face = StarShipPtr->ShipFacing;

	// The cyborg uses this function to aim its shots; it needs different variables
	if (PlayerControl[ShipPtr->playerNr] & CYBORG_CONTROL)
		MissileBlock.index = 0;
	else
		MissileBlock.index = 26;

	MissileBlock.sender = ShipPtr->playerNr;
	MissileBlock.flags = IGNORE_SIMILAR | NONSOLID | BACKGROUND_OBJECT;
	MissileBlock.pixoffs = MELNORME_OFFSET;
	MissileBlock.speed = PUMPUP_SPEED;
	MissileBlock.hit_points = 200;
	MissileBlock.damage = 0;
	MissileBlock.life = PUMPUP_LIFE;
	MissileBlock.blast_offs = 0;

	if (PlayerControl[ShipPtr->playerNr] & HUMAN_CONTROL)
		MissileBlock.preprocess_func = spawn_holo_trail;
	else
		MissileBlock.preprocess_func = NULL;

	TracerArray[0] = initialize_missile (&MissileBlock);

	if (TracerArray[0])
	{
		SIZE dx, dy;
		ELEMENT *MissilePtr;
		PRIMITIVE *lpPrim;

		LockElement (TracerArray[0], &MissilePtr);

		lpPrim = &(GLOBAL (DisplayArray))[MissilePtr->PrimIndex];

		// Hide holograms when the player isn't controlling the ship
		if (!(PlayerControl[MissilePtr->playerNr] & HUMAN_CONTROL))
		{
			SetPrimColor (lpPrim, BLACK_COLOR);
			SetPrimType (lpPrim, STAMPFILL_PRIM);
		}

		// Add some of the Trader's velocity to its projectiles
		GetCurrentVelocityComponents (&ShipPtr->velocity, &dx, &dy);
		dx = dx * 3/4;
		dy = dy * 3/4;
		DeltaVelocityComponents (&MissilePtr->velocity, dx, dy);
		MissilePtr->current.location.x -= VELOCITY_TO_WORLD (dx);
		MissilePtr->current.location.y -= VELOCITY_TO_WORLD (dy);
		UnlockElement (TracerArray[0]);
	}

	return (1);
}

static void
melnorme_postprocess (ELEMENT *ElementPtr)
{
	STARSHIP *StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	if ((StarShipPtr->cur_status_flags & SPECIAL)
			&& StarShipPtr->special_counter == 0
			&& DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST))
	{
		HELEMENT Confusion;

		initialize_confusion (ElementPtr, &Confusion);
		if (Confusion)
		{
			ELEMENT *CMissilePtr;
			LockElement (Confusion, &CMissilePtr);
			
			ProcessSound (SetAbsSoundIndex (
					StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1), CMissilePtr);
			
			UnlockElement (Confusion);
			PutElement (Confusion);
			StarShipPtr->special_counter =
					StarShipPtr->RaceDescPtr->characteristics.special_wait;
		}
	}

	// Toggle aim-assist
	if(StarShipPtr->cur_status_flags & DOWN
			&& !(StarShipPtr->old_status_flags & DOWN))
	{
		if (StarShipPtr->static_counter > 0)
			StarShipPtr->static_counter = 0;
		else
			StarShipPtr->static_counter = 1;
	}
	
	if (StarShipPtr->static_counter > 0 && StarShipPtr->auxiliary_counter == 0)
	{
		HELEMENT hTracer;

		initialize_tracer (ElementPtr, &hTracer);
				
		if (hTracer)
		{
			ELEMENT *TracePtr;

			LockElement (hTracer, &TracePtr);
			SetElementStarShip (TracePtr, StarShipPtr);
			UnlockElement (hTracer);

			PutElement (hTracer);
			StarShipPtr->auxiliary_counter = HOLO_WAIT + 1;
		}
	}
}

static void
melnorme_intelligence (ELEMENT *ShipPtr, EVALUATE_DESC *ObjectsOfConcern,
		COUNT ConcernCounter)
{
	BYTE old_count;
	STARSHIP *StarShipPtr;
	EVALUATE_DESC *lpEvalDesc;

	GetElementStarShip (ShipPtr, &StarShipPtr);

	StarShipPtr->RaceDescPtr->init_weapon_func = initialize_tracer;
	old_count = StarShipPtr->weapon_counter;

	if (StarShipPtr->weapon_counter == WEAPON_WAIT)
		StarShipPtr->weapon_counter = 0;

	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
	if (lpEvalDesc->ObjectPtr)
	{
		if (StarShipPtr->RaceDescPtr->ship_info.energy_level < SPECIAL_ENERGY_COST
				+ WEAPON_ENERGY_COST
				&& !(StarShipPtr->old_status_flags & WEAPON))
			lpEvalDesc->MoveState = ENTICE;
		else
		{
			STARSHIP *EnemyStarShipPtr;

			GetElementStarShip (lpEvalDesc->ObjectPtr, &EnemyStarShipPtr);
			if (!(EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags
					& IMMEDIATE_WEAPON))
				lpEvalDesc->MoveState = PURSUE;
		}
	}
	ship_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter);

	if (StarShipPtr->weapon_counter == 0
			&& (old_count != 0
			|| ((StarShipPtr->special_counter
			|| StarShipPtr->RaceDescPtr->ship_info.energy_level >= SPECIAL_ENERGY_COST
			+ WEAPON_ENERGY_COST)
			&& !(StarShipPtr->ship_input_state & WEAPON))))
		StarShipPtr->ship_input_state ^= WEAPON;

	StarShipPtr->ship_input_state &= ~SPECIAL;
	if (StarShipPtr->special_counter == 0
			&& StarShipPtr->RaceDescPtr->ship_info.energy_level >= SPECIAL_ENERGY_COST)
	{
		BYTE old_input_state;

		old_input_state = StarShipPtr->ship_input_state;

		StarShipPtr->RaceDescPtr->init_weapon_func = initialize_confusion;

		++ShipPtr->turn_wait;
		++ShipPtr->thrust_wait;
		ship_intelligence (ShipPtr, ObjectsOfConcern, ENEMY_SHIP_INDEX + 1);
		--ShipPtr->thrust_wait;
		--ShipPtr->turn_wait;

		if (StarShipPtr->ship_input_state & WEAPON)
		{
			StarShipPtr->ship_input_state &= ~WEAPON;
			StarShipPtr->ship_input_state |= SPECIAL;
		}

		StarShipPtr->ship_input_state = (unsigned char)(old_input_state
				| (StarShipPtr->ship_input_state & SPECIAL));
	}

	StarShipPtr->weapon_counter = old_count;

	StarShipPtr->RaceDescPtr->init_weapon_func = initialize_pump_up;
}

RACE_DESC*
init_melnorme (void)
{
	RACE_DESC *RaceDescPtr;

	melnorme_desc.postprocess_func = melnorme_postprocess;
	melnorme_desc.init_weapon_func = initialize_pump_up;
	melnorme_desc.cyborg_control.intelligence_func = melnorme_intelligence;
	
	RaceDescPtr = &melnorme_desc;

	return (RaceDescPtr);
}

