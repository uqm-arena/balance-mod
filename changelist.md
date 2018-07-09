==General==

* The "Down" key is unlocked. It does not perform any function normally, but commands may now be bound to it.

* All ship point values are set at even numbers.

::''This makes it easier to build a fleet up to an exact number, rather than a fleet of 197 or 199 when using a 200 point maximum limit. In practice, ships are scored between 1 and 15 rather than 1 and 30, but the old point range remains so that players can easily compare ship values between Balance Mod and vanilla UQM.''

* Many projectiles are now affected by the velocity of the ship they launch from, pushing toward the ship's direction of travel. The magnitude of this push varies between weapon systems. In most cases the difference is subtle.

::''The intent here is to help forward-mounted weapons be more useful when a ship is moving toward a target and less drastic when moving away.''

* When two of the same ship type fight, each ship will be highlighted by a colored reticle; green for the player's ship, red for the opponent's.

* There is a new game variant: Retreat. Ships can retreat once in combat if the retreat mode is enabled. To do this go in the game options "advanced" page and set the "S-Melee retreat" option to "once".

==Cyborg==

* Orz deploys marines against Earthling.

* Ur-Quan does not launch fighters against cloaked Ilwrath.

* Utwig and Yehat use their shields to defend against the Androsynth blazer.

''The Cyborg is not at all the focus of Balance Mod. Any improvements to artificial intelligence which do occur are made on a whim.''

==Androsynth Guardian==

* Point value raised from 15 to 20.

* Ship mass lowered from 6 to 5.

* Acid bubble duration decreased from 200 to 192.

* Blazer energy drain occurs on a 9 frame delay rather than an 8 frame delay. If an Androsynth activates its blazer form with full battery, it will have an additional second before its battery is empty.
::''This change was implemented to keep Androsynth effective against the new, buffed Spathi, which can be played in a way that forces Androsynth to run through its entire battery trying to ram into it while dodging the new and improved torpedoes.''

* The blazer leaves a visible comet trail.

* VUX limpets no longer attach to this ship while it is in blazer form.
::''This is a continuity change and does not affect the dynamic of Androsynth vs. VUX much at all (i.e. vanilla Androsynth still beat VUX by a landslide kept most of its value even while limpeted because limpets do not affect the blazer). It was silly that the blazer could wipe out Orz marines but not VUX limpets.''

* Androsynth's facing angle reverses instantly if the ship collides with a planet or shielded enemy while in blazer form, bouncing the ship in the opposite direction.
::''This lowers Androsynth's effectiveness against Yehat, which is now intended as a counter for Androsynth. The added planet bounce behavior is for the Androsynth player's benefit as it prevents a blazer from ramming a planet multiple times in a row.''

* Blazer collisions charge Utwig's absorption field the same as any other weapon.

''The super-efficient Androsynth needed to be nerfed badly. I favored a significant price increase to balance the ship rather than a performance downgrade. I made minor adjustments to some of its characteristics, but Androsnyth still plays very similarly to its old self.''

==Arilou Skiff==

* Speed increased from 40 to 44.

* Laser range decreased from 100 to 88.

* Teleporter energy cost increased from 3 to 4.

* The Skiff now has two different teleportation functions. Forward teleport is triggered by pressing "Up + Special", while an escape teleport will kick in when the Special key is pressed in absence of the "Up" key.

* Forward teleport moves the Skiff to a point that is forward from its current facing angle. Each destination point has a small amount of vertical and horizontal variation.

* Escape teleport moves the Skiff to a random location that is always far away from the Skiff's opponent.

* Teleporter transit time increased from 5 frames to 13. Melee's camera will give away Arilou's destination early into this cycle.

* All ship functions are disabled for 4 frames after teleportation, except for the tracking laser which is disabled for 13 frames.
::''Forward teleport, while a good buff for Arilou and very fun to play around with, was wildly overpowered until all these delays were added.''

* The Skiff may teleport again as it is reappearing, allowing for 100% safe fake-outs using forward teleport.
::''This helps against Shofixti a bit.''

* The Skiff's teleportation function has a safety mechanism which teleports the ship again if the ship warps into solid matter. This mechanism requires enough energy to teleport again or it will not trigger.
::''This change was implemented because there is no longer an advantage to chain teleporting for minutes on end, thus the threat of teleportation-related death is no longer necessary.''

''Vanilla Arilou depends on a technique known as "chain teleportation" to be useful in competitive PvP. It involves teleporting over and over until appearing at an ideal location to attack from. Nobody seemed especially fond of chain teleportation when I asked around the Star Control fan community and several veteran players were convinced that it ruined gameplay, so an overhaul of Arilou was necessary. The new forward teleport is meant to replace that technique with something similar, yet more fun. This variant is also noticeably stronger than the underpowered vanilla Arilou.''

==Chenjesu Broodhome==

* Point value lowered from 28 to 24.

* Ship mass decreased from 10 to 9.

* Photon shards are partially affected by the Broodhome's velocity.

* Shrapnel fragments now spiral outward and then back inward at a higher speed and longer duration than before. The spiral pattern's area of effect covers a similar area to vanilla's pattern.
::''This weapon's coverage and likelihood of hitting something is vastly improved.''

* The shrapnel graphic has been swapped out for a similar graphic that is 5x5 pixels rather than 4x4 at maximum zoom.

* Shrapnel damage decreased from 2 to 1.
::''New shrapnel hits targets within its area of effect more than twice as often. I nerfed the damage back down to keep the weapon from becoming too good. Why is buffing the weapon's coverage and nerfing its damage better for the game, you might ask? Because this version inflicts partial damage consistently, whereas vanilla Chenjesu's shrapnel could miss the broad side of a barn over and over.''

* Shrapnel no longer inflicts damage upon friendly DOGIs.
::''This prevents Chenjesu's improved shrapnel explosion from wiping out its own DOGIs left and right, which was a big problem during testing.''

''Vanilla Chenjesu is not really worth using in competitive PvP. The primary weapon's shrapnel explosion struck me as the best place to make improvements, as it was never very reliable. The new explosion indeed makes a significant difference. Even so, Chenjesu's ship price still needed to drop for the ship to be viable, and a price of 24 felt about right.''

==Chmmr Avatar==

* Ship mass decreased from 10 to 9.

::''The down-tweaks in ship mass you can find all over this change log are mostly for Chmmr's benefit. Lighter ships are more affected by the tractor beam. This particular adjustment is there just to keep Chmmr, Chenjesu, Ur-Quan, and Kohr-Ah at the same weight. The other three were reduced to give Chmmr bit more edge against the other ships.''

* Satellite laser color changed from deep blue to an even shade of red. The old beam was so dark that it was hard to see.

''Chmmr balance is pretty good in vanilla melee and barely needs any adjustment here. Many other ships are being buffed however, changing the dynamic they have against this one. Lowering ship masses was an easy way to give Chmmr a little extra push against certain ships. I'm also deliberately skewing Chmmr vs. Ur-Quan/Kohr-Ah towards Chmmr a bit more, as Chmmr is the easiest of the three 30pt ships to counter.''

==Druuge Mauler==

* Point value raised from 17 to 18.

* Mass driver is partially affected by the Mauler's velocity.

''Druuge is considered one of several "must-haves" for competitive vanilla melee, yet I felt it wasn't really worth more than its default price. With Mycon, Arilou, and Umgah (which is a long-shot soft-counter to Druuge) becoming more viable, there are better options to use against Druuge than before.''

==Earthling Cruiser==

* Point value raised from 11 to 12.

* Point-defense laser no longer fires upon planetary bodies.

* Fixed a defect present in vanilla UQM: Point-defense laser will no longer inflict 2 damage per strike against Umgah while that ship's antimatter cone is active.

''The default Earthling is already balanced and fun. It has been left alone for the most part.''

==Ilwrath Avenger==

* Top speed increased from 25 to 26.

::''This allows Ilwrath to catch Earthling with less difficulty during pursuit. Due to some quirk of the game's engine, you can sometimes get in a situation where both ships will be moving at full speed in the same direction, and the Ilwrath will not gain any ground on Earthling until they turn around and try to chase Earthling from another angle.''

* Battery capacity decreased from 16 to 12.

* Energy recovery delay increased from 4 frames to 6 frames.

::''Battery capacity and energy regeneration have been trimmed down substantially to mitigate the new additions this ship has received.''

* Hellfire velocity increased from 25 to 35. This increases the weapon's range.

* The Hellfire Spout has be redesigned to fire in three separate directions (forward, left diagonal, right diagonal) to give the weapon better coverage. The two diagonal flame spouts have a low rate of fire, so it's still best to attack your opponent head-on when possible. Diagonal spouts do not activate when attacking with a depleted battery. The energy expenditure of this weapon matches the number of projectiles fired.

::''What are the diagonal spouts for? Toasting dodgy ships that slip past your front spout. This makes a world of difference against Arilou, and is advantageous against short-range attackers such as Androsynth and Pkunk. In most match-ups these don't matter that much, but they still help Ilwrath out often enough that they're not a disadvantage.''

* The Avenger is now visible to its pilot while cloaked, appearing as a deep blue silhouette. A side effect of this is that the cloaked Avenger will show up on screen when two players go head-to-head on the same PC.

* Spontaneous re-facing towards an opponent no longer occurs when an Avenger fires its Hellfire Spout in a cloaked state.

* Cloaking Device energy cost decreased from 3 to 2.

''Balance Mod Ilwrath has received the royal treatment. Originally I began to tinker with this ship because a frequent sparring partner of mine had figured out how to consistently trick the ship's auto-turning capability (that thing that kicks in when a cloaked Ilwrath attacks) to miss its mark. Auto-turn was removed for this reason, but afterward Ilwrath proved to be hopeless without that capability, so I started trying out new features. I might have gotten a little carried away, but the new features are quite fun and the ship fills the same roles it once did in vanilla melee.''

==Kohr-Ah Marauder==

* Ship mass decreased from 10 to 9.

* Buzzsaws are partially affected by the Marauder's velocity while the primary weapon button is held down.

* Buzzsaw energy cost increased from 6 to 7.

::''These things are stupidly effective and this change should force Kohr-Ah players to ration out their energy more carefully.''

* FRIED cloud hitpoints decreased from 100 to 6.

::''Individual FRIED clouds can now be neutralized with enough firepower, though they will still melt every projectile they meet upon contact. 

''The downtweaks to Kohr-Ah's weapons should help rein it in. This ship was a just little too good in vanilla UQM.''

==Melnorme Trader==

* Point value raised from 18 to 20.

* Both Melnorme weapon systems are partially affected by the Trader's velocity.

* A blaster pulse suspended in front of the Trader will now break down correctly when hit by massive burst damage.

::''This is another continuity tweak. In vanilla melee these had to lose all their hitpoints twice over in an instant for them to break down. A suspended red pulse would inexplicably withstand another Melnorme Trader's in-motion red pulse rather than cancel out with it. Note the suspended red pulse behaves the same as before in all other circumstances.''

* Confusion pulse hitpoints lowered from 200 to 50.

::''What does 50 damage? Kohr-Ah FRIED, at least when it connects with another player's projectile. I gave Kohr-Ah--which has received some serious nerfs--a small new advantage against Melnorme.''

* Confusion disables the other player's "Down" key for its duration.

* Confusion disables Ur-Quan's autoturret for its duration.

''Melnorme has been priced up slightly due to its strong all-around performance at high level play. Ship characteristics are essentially the same.''

==Mmrnmhrm Transformer==

* Point value raised from 19 to 20.

* X-Form turn delay lowered from 2 to 1.

::''This gives Mmrnmhrm better results against flanking ships in particular.''

* Laser range set to 144. It is ever so slightly longer than before.

::''Someone figured out Slylandro could hit Mmrnmhrm from beyond range by a smidge. I did away with that.''

''Mmrnmhrm's point value at 18, 19, or 20 pts is pretty debatable. I went with 20 pts, and decided to accentuate its advantage against flanking ships to help justify that value. Teleport-backstabs from the new Arilou are somewhat of a threat to the original X-Form, and this change mitigates that.''

==Mycon Podship==

* Point value lowered from 21 to 16.

* Top speed decreased from 27 to 26.

::''This has an effect on Mycon vs. Kohr-Ah, allowing pursuit by Kohr-Ah to be a little easier. I haven't noticed an impact on any other match-up yet. It's possible this has been a detrimental tweak, but as a 16 pt ship that used to be 21 pt, I doubt it.''

''Vanilla Mycon is grossly overpriced. My preference to solve this was to drop the ship's price rather than improve its characteristics. The rationale here is that melee is a fighting game where damage inflicted is always permanent, with Mycon being the one exception, and it being able to heal itself means this ship in particular is at a high risk of becoming broken and ridiculous if any of its characteristics get a boost. Additionally, I felt Mycon was pretty fun exactly the way it was.''

==Orz Nemesis==

* Point value lowered from 23 to 22.

* Howitzer is partially affected by the Nemesis' velocity.

* Top speed increased from 35 to 36.

* Marines lead their target by a longer distance during pursuit.

* New feature: Marine Recall Signal. When used, all marines currently out in space turn green and retreat back to the Nemesis. The signal is triggered by pressing "Special + Down".

''With Androsynth's price raised to 20, Orz has become much more viable. Nevertheless, marines have been improved in a few ways because vanilla marines are underwhelming at high level play.''

==Pkunk Fury==

* Respawn chance begins at 80% and drops by 18% per respawn.

''The Fury's respawn ability has been tweaked to make Pkunk less random and generally more effective. You won't see 10+ respawns anymore, but it will respawn more often on average.''

==Shofixti Scout==

* Point value raised from 5 to 6.

* Mendokusai dart speed decreased from 96 to 92.

* Mendokusai dart duration decreased from 10 to 9.

* Mendokusai darts are partially affected by the Scout's velocity.

* Glory Device damage is halved. If the enemy ship is inside the area covered by the explosion, this weapon inflicts 8 damage flat as opposed to 6-9 depending on the exact distance. The Glory Device will otherwise inflict 0-5 damage outside of this area.

::''Note that the Glory Device is still threatening to a wide selection of ships even after being halved in potency. It is an effective finisher of many (if not most) ship types when they are heavily damaged.''

* The Glory Device is now negated by Yehat and Utwig shielding.

''Shofixti needed a serious beating from the nerf bat. It was far too deadly in a wide variety of different situations for 5 points. Furthermore, since the game is designed for the Glory Device-user to choose its next ship second after a mutual destruction, a kill from the Glory Device means a strong advantage on picking a counter ship for the next match too. The new Shofixti is indeed weak, and being the cheapest ship in the game, it's about where it should be.''

==Slylandro Probe==

* Point value raised from 17 to 18.

* The lightning weapon's maximum range has been capped. It is now roughly equivalent to the Chmmr, VUX or Mmrnmhrm lasers.

::''Slylandro lightning will zig-zag randomly while tracking toward the opposing ship. Sometimes the ship will unleash a lightning bolt which doesn't zig-zag very much at all, giving it unusually long range. The fluctuating weapon range could be abused by firing from outside an opponent's reach repeatedly until scoring enough long shots to beat them. This exploit would lead to boring, unbalanced combat.''

* Lightning duration is longer. The minimum distance a lightning streak will cover is a bit higher on average.

* Lightning seeks out enemy ships regardless of whether they are cloaked or not.

::''This change keeps Slylandro effective against the new Ilwrath, which has better weapon range and coverage.''

''Slylandro was consistently devastating in vanilla melee. With Spathi repurposed specifically to counter this ship, pricing Slylandro up higher seemed unnecessary. The lightning weapon's reach has been tweaked but if you've never tried to abuse vanilla Slylandro's theoretical maximum range then you won't even notice the difference.''

==Spathi Eluder==

* Point value lowered from 18 to 16.

* The front cannon now fires three projectiles forward in a slight spread. Projectile velocity is lower than before. This weapon consumes 3 energy per shot and has a 5 frame cooldown.

* Front cannon is significantly affected by the Eluder's velocity.

* Torpedo speed increased from 32 to 40.

* Torpedo duration reduced from 30 to 24.

* Torpedo initial homing delay removed. The weapon is more responsive against enemies at short range as a result.

''Spathi has been repurposed to act as a destroyer of flanking ships. This has been achieved by improving the ship's torpedoes. Once considered to be a poor fleet addition in PvP, this ship can now take on the dreaded Slylandro. The new cannon is a step up from before, but it is still relatively weak.''

==Supox Blade==

* Point value lowered from 16 to 14.

* Gob launcher is partially affected by the Blade's velocity.

* The "Down" key causes Supox to accelerate backwards. "Special + Up" no longer performs this function. Lateral thrust remains exactly the same.  The Blade may now accelerate in reverse and rotate at the same.

* Reverse or lateral acceleration causes exhaust to spawn.

* Battery size decreased from 16 to 12.

* Energy recovery delay increased from 4 to 5.

''I added the "Down" key to melee specifically to make Supox's reverse thrust ability more intuitive. Aside from that, Supox's sustainable firepower has been cut for the purpose of fitting the ship into a slightly lower point value.''

==Syreen Penetrator==

* Point value lowered from 13 to 12.

* Particle blaster is partially affected by the Penetrator's velocity.

* Siren song energy cost increased from 5 to 8.

* Siren song cooldown increased from 20 to 24.

* Siren song potency decreased from 8 to 6.

* Siren song potency against other Syreen Penetrator vessels is set to 2.

::''This was done to make the outcome of a Syreen mirror match less arbitrary. Both ships calling and gathering each other's entire crew while hurling shots at each other got pretty silly, usually leaving the winner with more crew than they started with.''

''Vanilla Syreen was thought to be overpowered, though not tremendously so. The siren song has been toned down considerably for the purpose of pushing the ship into the role of a medium-low value ship.''

==Thraddash Torch==

* Crew count increased from 8 to 10.

* Ship mass decreased from 5 to 4.

* Battery capacity decreased from 24 to 14.

* The Mark VI Blaster has been transformed from a long range "plinker" to a set of short range, rapid-fire guns. The new projectile has a velocity of 72 and a duration of 10 frames. Cooldown between shots is 3 frames. Two projectiles fire at a time, consuming 2 energy per shot. A new sound effect has been attached to this weapon.

* Blaster shots are significantly affected by the Torch's velocity.

* Afterburner will consume 2 energy per 3 frames of afterburn, lowering energy consumption.

* Afterburner flame puff hitpoints increased from 1 to 2.

* Afterburner flame puff duration increased from 48 to 96.

* Afterburner flame puff fade animation starts earlier and occurs more gradually.

''Star Control's most broken ship has been given much attention. The original Torch out-ranges most others with its blaster, and has a higher top speed than anything due to its afterburner; this allows it to slowly and painfully pick apart most other ships. The new blaster fixes this problem. The Torch is now somewhat flimsy, yet fast and versatile.''

==Umgah Drone==

* Point value raised from 7 to 8.

* Ship mass increased from 1 to 2.

* Antimatter cone substantially increased in size.

* Turn delay increased from 4 to 5.

* Battery capacity decreased from 30 to 24.

* Retropropulsion will consume energy for every other consecutive zip, halving energy consumption.

* Retropropulsion speed decreased from 160 to 120.

''The new antimatter cone is a big help for the Umgah Drone. Initially the larger cone was too powerful, and the ship's turn rate had to be lowered to compensate. Changes to retropropulsion were applied to make the ship easier for less skilled players to control, though Umgah remains one of the more challenging ships to play as.''

==Ur-Quan Dreadnought==

* Ship mass decreased from 10 to 9.

* Fusion bolt energy cost decreased from 6 to 5.

* Fusion bolts now launch in an alternating pattern between the front, left arm, and right arm of the Dreadnought. The pattern loops like this: Center-Left-Center-Right. If the Dreadnought's battery is full or empty, the pattern will reset back to center.

* Fusion cannons are significantly affected by the Dreadnought's velocity.

* Fighters are given a much longer timer of ~17 seconds to return to the Dreadnought before expiring once their attack duration is over.

* In addition to dodging planets, fighters also evade asteroids and enemy ships rather than crashing into them and dying.

* Fighters no longer die from friendly fusion bolts they come into contact with when making a return trip to the Dreadnought.

::''Note that attacking fighters can not be harmed by friendly fire in vanilla UQM. Because of this, I figured the vulnerability of returning fighters was a bug.''

* Fighters fan out from each other if they are too close together.

* The fighter's beam weapon has 64 firing angles rather than 16.

* The fighter's beam weapon connects instantly with its target.

* The fighter's beam weapon range is extended from 44 to 52.

* Fighter weapon cooldown increased from 8 to 24.

::''This may appear to be a crippling penalty at first glance. The vanilla Ur-Quan fighter is heavily impeded by firing angle limitations, a delay between firing and scoring a hit, and a likelihood of eventually crashing into the enemy ship and dying; all of these flaws have been rectified, and this change is needed to keep the new beam under control.''

* Fighters will attack with a different, energy-siphoning laser that kills 1 crew and drains 1 energy. This beam is light cyan in color and has a new sound effect and collision graphic.

::''This prevents Utwig--a ship which already excels against Kohr-Ah and Chmmr--from dominating Ur-Quan and improves fighter performance across the board.''

* Fighters are a light shade of yellow-green in color rather than brick red to match the Dreadnought itself.

* Autoturret added! This weapon periodically fires a single damage, super-accurate homing projectile at nearby threats if they are close enough. The effective range of this weapon is quite short. Small enemy projectiles are high priority targets, the enemy ship is a low priority target, and large projectiles such as Kohr-Ah buzzsaws are ignored. Autoturret cooldown is 22 frames. This weapon consumes one point of energy per shot.

::''This provides the Dreadnought with a small measure of short-range defense, which the other 30-pt ships have and Ur-Quan needed to remain effective at that cost. No more Dreadnoughts losing half their crew chasing down Earthling, or being hard-countered by Mmrnmhrm missile spam.''

''Ur-Quan has received a massive overhaul. Impressive as it may seem at first blush, the Dreadnought becomes seriously underpowered for its cost as player skill increases. Rather than price it down to 12-22 pts as Ur-Quan is likely worth in vanilla melee, it has received substantial buffs to its capabilities to bring it up to the level of Kohr-Ah and Chmmr. While the changes may initially seem extreme, #uqm-arena regulars agreed that large-scale changes were needed to keep it at 30 pts, though its overall feel and play style is not too different. Lowering the point value of this ship was not desirable for thematic reasons. This ship's intended hard counter is Orz, for anyone wondering; the space marines have enough hitpoints to withstand a few shots from the autoturret. A few other ships can achieve varying degrees of success against Ur-Quan as well.''

==Utwig Jugger==

* Point value raised from 22 to 24.

* Maximum battery size increased from 20 to 24. Starting energy remains at 10.

::''This improves effectiveness against Kohr-Ah and Chmmr, while having a negligible effect on all other match-ups.''

* Absorption field charges up normally during Androsynth blazer collisions. It will also bounce the blazer backwards.

* Absorption field negates and absorbs Shofixti Glory Device explosions.

* Absorption field no longer charges up by 10 points per frame when touching a Chmmr satellite. Although this was clearly a bug, removing it admittedly does not make much of a difference when Utwig and Chmmr are fighting as Chmmr satellites still rapidly charge up a shielded Utwig's battery.

* Energy lances are partially affected by the Jugger's velocity.

''Utwig is best known among veteran melee players as the killer of 30 pt ships which is itself easily countered. I've increased its price to make it less of a "must-have". Additionally, the new Ur-Quan is no longer a viable target for Utwig. Nerfs to Kohr-Ah and one subtle buff to Utwig should make the Utwig vs. Kohr-Ah match-up more approachable for non-expert players. Various changes aside, Utwig remains very similar to its vanilla counterpart.''

==VUX Intruder==

* Point value raised from 12 to 14.

* Battery capacity decreased from 40 to 34.

* Energy recovery delay decreased from 8 frames to 7 frames.

* The maximum distance an Intruder will spawn from an enemy ship is reduced, improving its "ambush" capability slightly. As before, VUX Intuders will rarely ever get the drop on an opponent which is already in motion at the start of combat.

* Limpet energy cost increased from 2 to 3.

* Limpet duration decreased from 80 to 72 frames.

* Limpet effect on enemy top speed and acceleration is diminished slightly.

* Limpet color adjusted to turquoise.

::''This makes them more visually distinct from floating crew during Vux vs. Syreen matches.''

* Limpets no longer attach to the Androsynth Guardian while it is in blazer form.

* New feature: Immobilization. Once an enemy ship's top speed is reduced to 8, that ship's other movement characteristics (turn delay, acceleration delay, acceleration increment) will also hit rock bottom and the ship will become almost totally immobilized. A different "chomp" sound effect has been attached to all limpet collisions that occur at or beyond this point of immobilization to make it obvious.

::''This feature was implemented as a time-saving mechanism. It would normally take as many as 100 limpets to effectively shut down a ship's turning capability. Fights the VUX has already won will end faster as a result.''

''VUX limpet nerfs are there to add some granularity to how impaired a ship is as it picks them up. Ships which are doomed the moment they pick up a single limpet (Utwig, Yehat) in vanilla UQM still are, while others put up a better fight. The laser is more usable due to increased energy recovery, making VUX play less one-dimensional. The VUX price raise coincides with Utwig's, which is widely considered VUX's favorite prey.''

==Yehat Terminator==

* Point value lowered from 23 to 18.

* Top speed increased from 30 to 32.

* Acceleration adjusted to build up more evenly toward the Terminator's new top speed.

* Twin pulse cannons are partially affected by the Terminator's velocity.

* Shield negates Shofixti Glory Device explosions.

* Shield bounces Androsynth's blazer form backwards.

* Shield no longer triggers redundantly while it is already in effect. Previously the shield system was very wasteful of energy if its button was held down.

* Shield energy cost is 3, duration is 16, and cooldown is 8.

* Energy recovery does not occur for the entire length of shield duration + shield cooldown.

''Vanilla Yehat was never very much fun in net melee. Any player familiar with the shield tapping technique could keep a fight going for a very long time. The new shield is more player-friendly, slightly less powerful and much less irritating. The ship's price has also plummeted as even the original Yehat was not worth its cost. Changes to weapon relativity have occurred across the board and Yehat in particular benefits from this, as its strafing gravity whip attacks are more effective.''

==Zoq-Fot-Pik Stinger==

* Price raised from 6 to 8.

* Top speed increased from 40 to 42.

* Scattergun is partially affected by the Stinger's velocity.

* Tongue length increased. The difference can only be discerned when the camera is fully zoomed in. All hit detection in Star Control is done based on full-zoom sprites whether you see them or not, so only the largest tongue image was altered.

''To increase the ship's power in general and reliability against Mycon in particular, the Stinger has been given a few uptweaks accompanied by a price increase. ZFP was not nearly as good as Shofixti in vanilla UQM. Price and stat changes between the two ships have hopefully evened things out a bit.''

