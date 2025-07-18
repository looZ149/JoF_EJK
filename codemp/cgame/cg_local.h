/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2005 - 2015, ioquake3 contributors
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#pragma once

#include "qcommon/q_shared.h"
#include "rd-common/tr_types.h"
#include "game/bg_public.h"
#include "cg_public.h"

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000

// Zoom vars
#define	ZOOM_TIME			150		// not currently used?
#define MAX_ZOOM_FOV		3.0f
#define ZOOM_IN_TIME		1500.0f
#define ZOOM_OUT_TIME		100.0f
#define ZOOM_START_PERCENT	0.3f

#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#define	TEXT_ICON_SPACE		4

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define NUM_FONT_BIG	1
#define NUM_FONT_SMALL	2
#define NUM_FONT_CHUNKY	3

#define	NUM_CROSSHAIRS	10

#define TEAM_OVERLAY_MAXNAME_WIDTH	32
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	64

#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

//[JAPRO - Clientside - All - Define cinfo bits - Start]
#define JAPLUS_CINFO_FLIPKICK			(1<<0)	//Allow player flipkicking
#define JAPLUS_CINFO_FIXROLL1			(1<<1)	//Grip whilst rolling (even fixroll 0 is not basejka, because of how backwards roll is activated)
#define JAPLUS_CINFO_FIXROLL2			(1<<2)	//Grip whilst rolling + chainable rolls
#define JAPLUS_CINFO_FIXROLL3			(1<<3)	//Long roll + breakable
#define JAPLUS_CINFO_YELLOWDFA			(1<<4)	//improve yellow dfa
#define JAPLUS_CINFO_HEADSLIDE			(1<<5)	//jp_slideonplayer set
#define JAPLUS_CINFO_SPATTACKS			(1<<6)	//allow sp attacks
#define JAPLUS_CINFO_NEWDFA				(1<<7)	//new close range dfa
#define JAPLUS_CINFO_MODELSCALE			(1<<8)	//allow modelscale
#define JAPLUS_CINFO_DMGSPEEDSCALE		(1<<9)	//allow damage speed scale for modelscale
#define JAPLUS_CINFO_MACROSCAN1			(1<<10)	//macroscan 1
#define JAPLUS_CINFO_MACROSCAN2			(1<<11)	//macroscan 2
#define JAPLUS_CINFO_JK2DFA				(1<<12)	//jk2 dfa
#define JAPLUS_CINFO_NOKATA				(1<<13)	//no kata
#define JAPLUS_CINFO_NO_AUTO_REPLIER	(1<<14)	//no auto replier
#define JAPLUS_CINFO_GLA_ANIMS			(1<<15)	//new gla anims
#define JAPLUS_CINFO_LEDGEGRAB			(1<<16)	//Ledgegrab allowed
#define JAPLUS_CINFO_ALTDIM				(1<<17)	//Any jp_altdim is set

#define RESTRICT_SB					(1<<0)	//remove hackbots
#define RESTRICT_COSBY				(1<<1)	//remove hackbots
#define RESTRICT_LEAD				(1<<2)	//remove lead indicator
#define RESTRICT_WH					(1<<3)	//remove brightskin wh
#define RESTRICT_YAW				(1<<4)	//enforce yawspeed cap
#define RESTRICT_PLUGIN				(1<<5)	//force plugin for racers
#define RESTRICT_PLAYERLABELS		(1<<6)	//
#define RESTRICT_FLIPKICKBIND		(1<<7)	//
#define RESTRICT_STRAFETRAIL		(1<<8)	//force plugin for racers
#define RESTRICT_DO					(1<<9)	//force plugin for racers

//Style playermodel stuff
#define JAPRO_STYLE_FULLBRIGHT				(1<<0)
#define JAPRO_STYLE_SHELL					(1<<1)
#define JAPRO_STYLE_HIDEDUELERS1			(1<<2)
#define JAPRO_STYLE_HIDERACERS1				(1<<3) //hide racer if we ffa
#define JAPRO_STYLE_HIDERACERS2				(1<<4) //hide ffa if we racer
#define JAPRO_STYLE_HIDERACERS3				(1<<5) //hide racers if we racer
#define JAPRO_STYLE_RACERVFXDISABLE			(1<<6) //show fellow racers normally
#define JAPRO_STYLE_NONRACERVFXDISABLE		(1<<7) //show racers normally
#define JAPRO_STYLE_VFXDUELERS				(1<<8)
#define JAPRO_STYLE_VFXALTDIM				(1<<9) //ja+
#define JAPRO_STYLE_HIDENONDUELERS			(1<<10) //base
#define JAPRO_STYLE_HIDEYSALSHELL			(1<<11)
#define	JAPRO_STYLE_PLAYERLOD				(1<<12)
#define JAPRO_STYLE_NOBODIES				(1<<13)
#define JAPRO_STYLE_NOFADESFX				(1<<14)
#define JAPRO_STYLE_COLOREDSPAWNBUBBLE		(1<<15)
#define JAPRO_STYLE_HIDECOSMETICS			(1<<16)
#define JAPRO_STYLE_DISABLEBREATHING		(1<<17)
#define JAPRO_STYLE_OLDGRAPPLELINE			(1<<18)
#define JAPRO_STYLE_ENABLE_ALTERNATEPOSE	(1<<19)
#define JAPRO_STYLE_FORCE_ALTERNATEPOSE		(1<<20)
#define JAPRO_STYLE_SEASONALCOSMETICS       (1<<21)

//japro ignore race fx
#define RS_TIMER_START					(1<<0) //Ignore sound for start trigger
#define BODY_FADE_TIME					(60000)

//Cosmetics
#define	JAPRO_COSMETIC_SANTAHAT	(1<<0)
#define	JAPRO_COSMETIC_PUMKIN	(1<<1)
#define	JAPRO_COSMETIC_CAP		(1<<2)
#define	JAPRO_COSMETIC_FEDORA	(1<<3)
#define	JAPRO_COSMETIC_CRINGE	(1<<4)
#define	JAPRO_COSMETIC_SOMBRERO	(1<<5)
#define	JAPRO_COSMETIC_TOPHAT	(1<<6)
//#define JAPRO_COSMETIC_FIRE		(1<<7)

//#define JAPRO_CINFO_UNLAGGEDPUSHPULL (1<<19)	//push pull unlagged

//JAPRO - Serverside + clientside , restrictions
#define JAPRO_RESTRICT_BHOP			(1<<0)
#define JAPRO_RESTRICT_CROUCHJUMP	(1<<1)
#define JAPRO_RESTRICT_DOUBLEJUMP	(1<<2)
//[JAPRO - Clientside - All - Define cinfo bits - End]

#define JAPRO_CHATLOG_ENABLE		(1<<0)
#define JAPRO_CHATLOG_SYNC			(1<<1)
#define JAPRO_CHATLOG_OLDTIMESTAMP	(1<<2)
#define JAPRO_CHATLOG_PRINT			(1<<3)
#define JAPRO_CHATLOG_CENTERPRINT	(1<<3)

typedef enum //server mod enum
{
	SVMOD_BASEJKA,
	SVMOD_BASEENHANCED,
	SVMOD_OJKALT,
	SVMOD_JAPLUS,
	SVMOD_JAPRO,
} serverMod_t;

typedef enum {
	FOOTSTEP_STONEWALK,
	FOOTSTEP_STONERUN,
	FOOTSTEP_METALWALK,
	FOOTSTEP_METALRUN,
	FOOTSTEP_PIPEWALK,
	FOOTSTEP_PIPERUN,
	FOOTSTEP_SPLASH,
	FOOTSTEP_WADE,
	FOOTSTEP_SWIM,
	FOOTSTEP_SNOWWALK,
	FOOTSTEP_SNOWRUN,
	FOOTSTEP_SANDWALK,
	FOOTSTEP_SANDRUN,
	FOOTSTEP_GRASSWALK,
	FOOTSTEP_GRASSRUN,
	FOOTSTEP_DIRTWALK,
	FOOTSTEP_DIRTRUN,
	FOOTSTEP_MUDWALK,
	FOOTSTEP_MUDRUN,
	FOOTSTEP_GRAVELWALK,
	FOOTSTEP_GRAVELRUN,
	FOOTSTEP_RUGWALK,
	FOOTSTEP_RUGRUN,
	FOOTSTEP_WOODWALK,
	FOOTSTEP_WOODRUN,

	FOOTSTEP_TOTAL
} footstep_t;

typedef enum {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct lerpFrame_s {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	qboolean	lastFlip; //if does not match torsoFlip/legsFlip, restart the anim.

	int			lastForcedFrame;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	float		yawSwingDif;

	int			animationNumber;
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact

	float		animationSpeed;		// scale the animation speed
	float		animationTorsoSpeed;

	qboolean	torsoYawing;
} lerpFrame_t;


typedef struct playerEntity_s {
	lerpFrame_t		legs, torso, flag;
	int				painTime;
	int				painDirection;	// flip from 0 to 1
	int				lightningFiring;

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;
} playerEntity_t;

//=================================================

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_COMBAT_SOUNDS	40
#define	MAX_CUSTOM_EXTRA_SOUNDS	40
#define	MAX_CUSTOM_JEDI_SOUNDS	40
// MAX_CUSTOM_SIEGE_SOUNDS defined in bg_public.h
#define MAX_CUSTOM_DUEL_SOUNDS	40

#define	MAX_CUSTOM_SOUNDS	40 //rww - Note that for now these must all be the same, because of the way I am
							   //cycling through them and comparing for custom sounds.

#define DEFAULT_FEMALE_SOUNDPATH "chars/mp_generic_female/misc"//"chars/tavion/misc"

#define DEFAULT_MALE_SOUNDPATH "chars/mp_generic_male/misc"//"chars/kyle/misc"

typedef struct clientInfo_s {
	qboolean		infoValid;

	float			colorOverride[3];

	saberInfo_t		saber[MAX_SABERS];
	void			*ghoul2Weapons[MAX_SABERS];

	char			saberName[MAX_QPATH];
	char			saber2Name[MAX_QPATH];

	char			name[MAX_QPATH];
	char			cleanname[MAX_QPATH];
	team_t			team;

	int				duelTeam;

	int				botSkill;		// -1 = not bot, 0-5 = bot

	int				frame;

	vec3_t			color1;
	vec3_t			color2;

	int				icolor1;
	int				icolor2;

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				armor;
	int				curWeapon;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				teamTask;		// task in teamplay (offence/defence)
	qboolean		teamLeader;		// true when this is a team leader

	int				powerups;		// so can display quad/flag status

	int				medkitUsageTime;

	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
//	char			headModelName[MAX_QPATH];
//	char			headSkinName[MAX_QPATH];
	char			forcePowers[MAX_QPATH];

	char			teamName[MAX_TEAMNAME];

	int				corrTime;

	vec3_t			lastHeadAngles;
	int				lookTime;

	int				brokenLimbs;

	qboolean		deferred;

	qboolean		newAnims;		// true if using the new mission pack animations
	qboolean		fixedlegs;		// true if legs yaw is always the same as torso yaw
	qboolean		fixedtorso;		// true if torso never changes yaw

	vec3_t			headOffset;		// move head in icon views
	//footstep_t		footsteps;
	gender_t		gender;			// from model

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	//qhandle_t		headModel;
	//qhandle_t		headSkin;

	void			*ghoul2Model;

	qhandle_t		modelIcon;

	qhandle_t		bolt_rhand;
	qhandle_t		bolt_lhand;

	qhandle_t		bolt_head;

	qhandle_t		bolt_motion;

	qhandle_t		bolt_llumbar;

	int				siegeIndex;
	int				siegeDesiredTeam;

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];
	sfxHandle_t		combatSounds[MAX_CUSTOM_COMBAT_SOUNDS];
	sfxHandle_t		extraSounds[MAX_CUSTOM_EXTRA_SOUNDS];
	sfxHandle_t		jediSounds[MAX_CUSTOM_JEDI_SOUNDS];
	sfxHandle_t		siegeSounds[MAX_CUSTOM_SIEGE_SOUNDS];
	sfxHandle_t		duelSounds[MAX_CUSTOM_DUEL_SOUNDS];

	int				legsAnim;
	int				torsoAnim;

	float		facial_blink;		// time before next blink. If a minus value, we are in blink mode
	float		facial_frown;		// time before next frown. If a minus value, we are in frown mode
	float		facial_aux;			// time before next aux. If a minus value, we are in aux mode

	int			superSmoothTime; //do crazy amount of smoothing
	vec3_t		rgb1, rgb2;//rgb sabers, use different ones for strafetrails. oh no.

	unsigned int	cosmetics;

#define _STRAFETRAILS 0
#if _STRAFETRAILS
	vec3_t		rgb1, rgb2; //rename this shit so it doesnt conflict with saber rgb
#endif

	int			deaths; //counted locally client-side, incase the server doesn't send this information already

	qboolean	useAlternateStandAnim;
} clientInfo_t;

//rww - cheap looping sound struct
#define MAX_CG_LOOPSOUNDS 8

typedef struct cgLoopSound_s {
	int entityNum;
	vec3_t origin;
	vec3_t velocity;
	sfxHandle_t sfx;
} cgLoopSound_t;

// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	// This comment below is correct, but now m_pVehicle is the first thing in bg shared entity, so it goes first. - AReis
	//rww - entstate must be first, to correspond with the bg shared entity structure
	entityState_t	currentState;	// from cg.frame
	playerState_t	*playerState;	//ptr to playerstate if applicable (for bg ents)
	Vehicle_t		*m_pVehicle; //vehicle data
	void			*ghoul2; //g2 instance
	int				localAnimIndex; //index locally (game/cgame) to anim data for this skel
	vec3_t			modelScale; //needed for g2 collision

	//from here up must be unified with bgEntity_t -rww

	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;
//	int				teleportFlag;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				dustTrailTime;
	int				miscTime;

	vec3_t			damageAngles;
	int				damageTime;

	int				snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t	pe;

//	int				errorTime;		// decay the error from this time
//	vec3_t			errorOrigin;
//	vec3_t			errorAngles;

//	qboolean		extrapolated;	// false if origin / angles is an interpolation
//	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;

#define RAGOFFSET_THING 1
#if 0
	vec3_t			lerpOriginOffset; //bucky
	//add up bone offsets until next client frame before adding them in
	qboolean		hasRagOffset;
	vec3_t			ragOffsets;
	int				ragOffsetTime;
#endif

	vec3_t			ragLastOrigin;
	int				ragLastOriginTime;

	qboolean		noLumbar; //if true only do anims and things on model_root instead of lower_lumbar, this will be the case for some NPCs.
	qboolean		noFace;

	//For keeping track of the current surface status in relation to the entitystate surface fields.
	int				npcLocalSurfOn;
	int				npcLocalSurfOff;

	int				eventAnimIndex;

	clientInfo_t	*npcClient; //dynamically allocated - always free it, and never stomp over it.

	int				weapon;

	void			*ghoul2weapon; //rww - pointer to ghoul2 instance of the current 3rd person weapon

	float			radius;
	int				boltInfo;

	//sometimes used as a bolt index, but these values are also used as generic values for clientside entities
	//at times
	int				bolt1;
	int				bolt2;
	int				bolt3;
	int				bolt4;

	float			bodyHeight;

	int				torsoBolt;

	vec3_t			turAngles;

	vec3_t			frame_minus1;
	vec3_t			frame_minus2;

	int				frame_minus1_refreshed;
	int				frame_minus2_refreshed;

#if _STRAFETRAILS
	vec3_t			frame_minus12, frame_minus22;
	int				frame_minus1_refreshed2, frame_minus2_refreshed2;
#endif

	void			*frame_hold; //pointer to a ghoul2 instance

	int				frame_hold_time;
	int				frame_hold_refreshed;

	void			*grip_arm; //pointer to a ghoul2 instance

	int				trickAlpha;
	int				trickAlphaTime;

	int				teamPowerEffectTime;
	int				teamPowerType; //0 regen, 1 heal, 2 drain, 3 absorb

	int				lastRagTime;
	qboolean		isRagging;
	qboolean		ownerRagging;
	int				overridingBones;

	int				bodyFadeTime;
	vec3_t			pushEffectOrigin;

	cgLoopSound_t	loopingSound[MAX_CG_LOOPSOUNDS];
	int				numLoopingSounds;

	int				serverSaberHitIndex;
	int				serverSaberHitTime;
	qboolean		serverSaberFleshImpact; //true if flesh, false if anything else.

	qboolean		ikStatus;

	qboolean		saberWasInFlight;

	float			smoothYaw;

	int				uncloaking;
	qboolean		cloaked;

	int				vChatTime;
#if 1
	vec3_t			lastOrigin; //strafetrail
	int				lastStrafeTrailTime;

	int				breathPuffTime;
	int				breathTime; //can maybe just use breathPuffTime from ci?
#endif

#if 1 //NPCLAGFIX2 adds this
	qboolean		doLerp; // for entity position smoothing
#endif
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independently from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FADE_SCALE_MODEL, // currently only for Demp2 shock sphere
	LE_FRAGMENT,
	LE_PUFF,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SCOREPLUM,
	LE_OLINE,
	LE_SHOWREFENTITY,
	LE_LINE,
	LE_MISSILE//japro
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE = 0x0001,			// do not scale size over time
	LEF_TUMBLE			= 0x0002,			// tumble over time, used for ejecting shells
	LEF_FADE_RGB		= 0x0004,			// explicitly fade
	LEF_NO_RANDOM_ROTATE= 0x0008			// MakeExplosion adds random rotate which could be bad in some cases
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS,
	LEBS_METAL,
	LEBS_ROCK
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect
	int				bounceSound;		// optional sound index to play upon bounce

	float			alpha;
	float			dalpha;

	int				forceAlpha;

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	union {
		struct {
			float radius;
			float dradius;
			vec3_t startRGB;
			vec3_t dRGB;
		} sprite;
		struct {
			float width;
			float dwidth;
			float length;
			float dlength;
			vec3_t startRGB;
			vec3_t dRGB;
		} trail;
		struct {
			float width;
			float dwidth;
			// Below are bezier specific.
			vec3_t			control1;				// initial position of control points
			vec3_t			control2;
			vec3_t			control1_velocity;		// initial velocity of control points
			vec3_t			control2_velocity;
			vec3_t			control1_acceleration;	// constant acceleration of control points
			vec3_t			control2_acceleration;
		} line;
		struct {
			float width;
			float dwidth;
			float width2;
			float dwidth2;
			vec3_t startRGB;
			vec3_t dRGB;
		} line2;
		struct {
			float width;
			float dwidth;
			float width2;
			float dwidth2;
			float height;
			float dheight;
		} cylinder;
		struct {
			float width;
			float dwidth;
		} electricity;
		struct
		{
			// fight the power! open and close brackets in the same column!
			float radius;
			float dradius;
			qboolean (*thinkFn)(struct localEntity_s *le);
			vec3_t	dir;	// magnitude is 1, but this is oldpos - newpos right before the
							//particle is sent to the renderer
			// may want to add something like particle::localEntity_s *le (for the particle's think fn)
		} particle;
		struct
		{
			qboolean	dontDie;
			vec3_t		dir;
			float		variance;
			int			delay;
			int			nextthink;
			qboolean	(*thinkFn)(struct localEntity_s *le);
			int			data1;
			int			data2;
		} spawner;
		struct
		{
			float radius;
		} fragment;
	} data;

	refEntity_t		refEntity;
} localEntity_t;


#define _NEWTRAILS 1
#if _NEWTRAILS

typedef struct strafeTrailRef_s {
	union {
		struct {
			float radius;
			float dradius;
			vec3_t startRGB;
			vec3_t dRGB;
		} sprite;
		struct {
			float width;
			float dwidth;
			float length;
			float dlength;
			vec3_t startRGB;
			vec3_t dRGB;
		} trail;
		struct {
			float width;
			float dwidth;
			// Below are bezier specific.
			vec3_t			control1;				// initial position of control points
			vec3_t			control2;
			vec3_t			control1_velocity;		// initial velocity of control points
			vec3_t			control2_velocity;
			vec3_t			control1_acceleration;	// constant acceleration of control points
			vec3_t			control2_acceleration;
		} line;
		struct {
			float width;
			float dwidth;
			float width2;
			float dwidth2;
			vec3_t startRGB;
			vec3_t dRGB;
		} line2;
		struct {
			float width;
			float dwidth;
			float width2;
			float dwidth2;
			float height;
			float dheight;
		} cylinder;
		struct {
			float width;
			float dwidth;
		} electricity;
		struct
		{
			// fight the power! open and close brackets in the same column!
			float radius;
			float dradius;
			qboolean (*thinkFn)(struct localEntity_s *le);
			vec3_t	dir;	// magnitude is 1, but this is oldpos - newpos right before the
							//particle is sent to the renderer
			// may want to add something like particle::localEntity_s *le (for the particle's think fn)
		} particle;
		struct
		{
			qboolean	dontDie;
			vec3_t		dir;
			float		variance;
			int			delay;
			int			nextthink;
			qboolean	(*thinkFn)(struct localEntity_s *le);
			int			data1;
			int			data2;
		} spawner;
		struct
		{
			float radius;
		} fragment;
	} data;

	refEntity_t		refEntity;
} strafeTrailRef_t;

typedef struct strafeTrail_s {
	struct strafeTrail_s	*prev, *next;
	short clientNum;
	//float radius;
	vec3_t start;
	vec3_t end;
	int endTime;
	unsigned int color;//More memory, but saves cycles not having to calculate it every frame based on clientnum or w/e.  also good to separate it from clientnum i guess for future features
} strafeTrail_t;
#endif

//======================================================================


typedef struct score_s {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;
	int				powerUps;
	int				accuracy;
	int				impressiveCount;
	int				excellentCount;
	int				gauntletCount;
	int				defendCount;
	int				assistCount;
	int				captures;
	int				deaths; //JAPRO - Scoreboard Deaths
	qboolean		perfect;
	int				team;
} score_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;		// this is the pickup model
	qhandle_t		viewModel;			// this is the in-view model used by the player
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	vec3_t			flashDlightColor;

	qhandle_t		weaponIcon;
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose
	sfxHandle_t		firingSound;
	sfxHandle_t		chargeSound;
	fxHandle_t		muzzleEffect;
	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;
	int				missileRenderfx;
	sfxHandle_t		missileHitSound;

	sfxHandle_t		altFlashSound[4];
	sfxHandle_t		altFiringSound;
	sfxHandle_t		altChargeSound;
	fxHandle_t		altMuzzleEffect;
	qhandle_t		altMissileModel;
	sfxHandle_t		altMissileSound;
	void			(*altMissileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			altMissileDlight;
	vec3_t			altMissileDlightColor;
	int				altMissileRenderfx;
	sfxHandle_t		altMissileHitSound;

	sfxHandle_t		selectSound;

	sfxHandle_t		readySound;
	float			trailRadius;
	float			wiTrailTime;

} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct itemInfo_s {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
/*
Ghoul2 Insert Start
*/
	void			*g2Models[MAX_ITEM_MODELS];
	float			radius[MAX_ITEM_MODELS];
/*
Ghoul2 Insert End
*/
} itemInfo_t;


typedef struct powerupInfo_s {
	int				itemNum;
} powerupInfo_t;


#define MAX_SKULLTRAIL		10

typedef struct skulltrail_s {
	vec3_t positions[MAX_SKULLTRAIL];
	int numpositions;
} skulltrail_t;


#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16

#define	MAX_EMOJI_LENGTH			24 //max length of the characters between the colons
#define	MAX_LOADABLE_EMOJIS			256 //max png files it can load
#define	MAX_CHATBOX_ITEM_EMOJIS		32 //max emojis per chat message
typedef struct loadableEmoji_s
{
	char		name[MAX_EMOJI_LENGTH];
	qhandle_t	emoji;
} loadableEmoji_t;

loadableEmoji_t emojis[MAX_LOADABLE_EMOJIS];

typedef struct chatBoxEmoji_s
{
	qhandle_t	emoji;
	int			yOffset;
	int			xOffset;
} chatBoxEmoji_t;

#define	MAX_CHATBOX_ITEMS		24
typedef struct chatBoxItem_s
{
	char			string[MAX_STRING_CHARS];
	int				time;
	int				lines;
	chatBoxEmoji_t emoji[MAX_CHATBOX_ITEM_EMOJIS];
} chatBoxItem_t;

#define	MAX_CLIENT_SPEEDPOINTS		32
typedef struct clientSpeedpoint_s
{
	int		speed;
	qboolean isSet;
	qboolean reached;
} clientSpeedpoint_t;

#define	MAX_CLIENT_CHECKPOINTS		32
typedef struct clientCheckpoint_s {
	int		x1, y1, z1, x2, y2, z2;
	qboolean isSet;
} clientCheckpoint_t;

typedef struct cg_s {
	int			clientFrame;		// incremented each frame

	int			clientNum;

	qboolean	demoPlayback;
//#define phoondemotech //later
#ifdef phoondemotech
	char    demoTimeStamp[32];
#endif
	qboolean	levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
//	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	mMapChange;

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	qboolean	mapRestart;			// set on a map restart to set back the weapon

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	playerState_t	predictedVehicleState;

	//centity_t		predictedPlayerEntity;
	//rww - I removed this and made it use cg_entities[clnum] directly.

	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;

	short		lastWeaponSelect[2];//japro

	int			forceSelect;
	int			itemSelect;

	// auto rotating items
	vec3_t		autoAngles;
	matrix3_t	autoAxis;
	vec3_t		autoAnglesFast;
	matrix3_t	autoAxisFast;

	// view rendering
	refdef_t	refdef;

	// zoom key
	qboolean	zoomed;
	int			zoomTime;

	int			damageTaken[32];

	qboolean	coldBreathEffects;
	qboolean	rainSoundEffects;

	float		zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int				scoresRequestTime;
	int				numScores;
	int				selectedScore;
	int				teamScores[2];
	score_t			scores[MAX_CLIENTS];
	qboolean		showScores;
	qboolean		scoreBoardShowing;
	int				scoreFadeTime;
	char			killerName[MAX_NETNAME];
	char			spectatorList[MAX_STRING_CHARS];		// list of names
	int				spectatorLen;												// length of list
	float			spectatorWidth;											// width in device units
	int				spectatorTime;											// next time to offset
	int				spectatorPaintX;										// current paint x
	int				spectatorPaintX2;										// current paint x
	int				spectatorOffset;										// current offset from start
	int				spectatorPaintLen; 									// current offset from start

	// skull trails
	skulltrail_t	skulltrails[MAX_CLIENTS];

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[MAX_STRING_CHARS];
	int			centerPrintLines;

	//idk
	int			oldammo;
	int			oldAmmoTime;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	// kill timers for carnage reward
	int			lastKillTime;

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

	int			crosshairVehNum;
	int			crosshairVehTime;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;
	int			voiceTime;

	// reward medals
	int			rewardStack;
	int			rewardTime;
	int			rewardCount[MAX_REWARDSTACK];
	qhandle_t	rewardShader[MAX_REWARDSTACK];
	qhandle_t	rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

	// for voice chat buffer
	int			voiceChatTime;
	int			voiceChatBufferIn;
	int			voiceChatBufferOut;

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	int			kick_time;
	vec3_t		kick_origin;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;
	int     nextOrbitTime;

	//qboolean cameraMode;		// if rendering from a loaded camera
	int			loadLCARSStage;

	int			forceHUDTotalFlashTime;
	int			forceHUDNextFlashTime;
	qboolean	forceHUDActive;				// Flag to show force hud is off/on

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

	int			VHUDFlashTime;
	qboolean	VHUDTurboFlag;

	// HUD stuff
	float			HUDTickFlashTime;
	qboolean		HUDArmorFlag;
	qboolean		HUDHealthFlag;
	qboolean		iconHUDActive;
	float			iconHUDPercent;
	float			iconSelectTime;
	float			invenSelectTime;
	float			forceSelectTime;

	vec3_t			lastFPFlashPoint;

/*
Ghoul2 Insert Start
*/
	int				testModel;
	// had to be moved so we wouldn't wipe these out with the memset - these have STL in them and shouldn't be cleared that way
	snapshot_t	activeSnapshots[2];
/*
Ghoul2 Insert End
*/

	// used for communication with the engine
	union {
		char						raw[MAX_CG_SHARED_BUFFER_SIZE];
		TCGPointContents			pointContents;
		TCGVectorData				vectorData;
		TCGGetBoltData				getBoltData;
		TCGTrace					trace;
		TCGG2Mark					g2Mark;
		TCGImpactMark				impactMark;
		ragCallbackDebugBox_t		rcbDebugBox;
		ragCallbackDebugLine_t		rcbDebugLine;
		ragCallbackBoneSnap_t		rcbBoneSnap;
		ragCallbackBoneInSolid_t	rcbBoneInSolid;
		ragCallbackTraceLine_t		rcbTraceLine;
		TCGMiscEnt					miscEnt;
		TCGIncomingConsoleCommand	icc;
		autoMapInput_t				autoMapInput;
		TCGCameraShake				cameraShake;
	} sharedBuffer;

	short				radarEntityCount;
	short				radarEntities[MAX_CLIENTS+16];

	short				bracketedEntityCount;
	short				bracketedEntities[MAX_CLIENTS+16];

	float				distanceCull;

	chatBoxItem_t		chatItems[MAX_CHATBOX_ITEMS];
	int					chatItemActive;

#if 0
	int					snapshotTimeoutTime;
#endif

	qboolean spawning;
	int	numSpawnVars;
	char *spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
	int numSpawnVarChars;
	char spawnVarChars[MAX_SPAWN_VARS_CHARS];

	clientCheckpoint_t	clientCheckpoints[MAX_CLIENT_CHECKPOINTS];//japro checkpoints
	clientSpeedpoint_t	clientSpeedpoints[MAX_CLIENT_SPEEDPOINTS];//japro speedpoints
	int					lastCheckPointPrintTime;
	int					timerStartTime;
	int					lastGroundTime;//japro
	qboolean			firstTimeInAir;
	float				lastGroundSpeed;
	float				lastZSpeed;
	int					lastJumpHeightTime;//japro
	float				lastJumpHeight;
	int					lastStartTime;//japro
	float				lastYawSpeed;
	qboolean			recording;
	unsigned int		displacement;
	unsigned int		displacementSamples;
	int					maxSpeed;
	int					lastRaceTime;
	float				currentSpeed;
	int					startSpeed;
	float				previousSpeed;
	float				lastJumpDistance;
	int					lastJumpDistanceTime;
	qboolean			wasOnGround;
	vec3_t				lastGroundPosition;

	int					telemarkX;//japro
	int					telemarkY;//japro
	int					telemarkZ;//japro
	int					telemarkYaw;//japro
	int					lastAutoKillTime;

	//int				predictedRocketJumpTime;
	//int				predictedRocketJumpExpireTime;
	//vec3_t			predictedRocketJumpOriginalVel;
	vec3_t				predictedRocketJumpImpulse;
	qboolean			predictKnockback;

	float				lastXpos;
	float				lastYpos;

	vec4_t				strafeHelperActiveColor;
	vec4_t				crosshairColor;

	int					lowjumpTime;
	int					norollTime;

	#define				MAX_DO_BUFFERS 16
	int					doVstrTime[MAX_DO_BUFFERS];
	char				doVstr[MAX_DO_BUFFERS][MAX_STRING_CHARS];

	short				numFKFrames;
	short				numJumps;
	int					userinfoUpdateDebounce;
	char				lastChatMsg[MAX_SAY_TEXT + MAX_NETNAME + 32];

	int					drawingStrafeTrails;//optimization i guess
	qboolean			loggingStrafeTrail;
	char				logStrafeTrailFilename[MAX_QPATH];
	fileHandle_t		strafeTrailFileHandle;
	float				predictedTimeFrac;	// frameInterpolation * (next->commandTime - prev->commandTime)

#if 0
	int					snapshotTimeoutTime;
#endif

	struct {//chatlog
		fileHandle_t	file;
		qboolean		started;
	} log;

} cg_t;

#define CAMERA_MIN_FPS 15

#define MAX_TICS	14

typedef struct forceTicPos_s
{
	int				x;
	int				y;
	int				width;
	int				height;
	char			*file;
	qhandle_t		tic;
} forceTicPos_t;
extern forceTicPos_t forceTicPos[];
extern forceTicPos_t ammoTicPos[];

typedef struct cgscreffects_s
{
	float		FOV;
	float		FOV2;

	float		shake_intensity;
	int			shake_duration;
	int			shake_start;

	float		music_volume_multiplier;
	int			music_volume_time;
	qboolean	music_volume_set;
} cgscreffects_t;

extern cgscreffects_t cgScreenEffects;

void CGCam_Shake( float intensity, int duration );
void CGCam_SetMusicMult( float multiplier, int duration );

enum
{
	CHUNK_METAL1 = 0,
	CHUNK_METAL2,
	CHUNK_ROCK1,
	CHUNK_ROCK2,
	CHUNK_ROCK3,
	CHUNK_CRATE1,
	CHUNK_CRATE2,
	CHUNK_WHITE_METAL,
	NUM_CHUNK_TYPES
};
#define NUM_CHUNK_MODELS	4

// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct cgMedia_s {
	qhandle_t	charsetShader;
	qhandle_t	whiteShader;

	qhandle_t	loadBarLED;
	qhandle_t	loadBarLEDCap;
	qhandle_t	loadBarLEDSurround;

	qhandle_t	bryarFrontFlash;
	qhandle_t	greenFrontFlash;
	qhandle_t	lightningFlash;

	qhandle_t	itemHoloModel;
	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	neutralFlagModel;

	qhandle_t	teamStatusBar;

	qhandle_t	deferShader;

	qhandle_t	radarShader;
	qhandle_t	siegeItemShader;
	qhandle_t	mAutomapPlayerIcon;
	qhandle_t	mAutomapRocketIcon;
	qhandle_t	minimapShader;//JAPRO

	qhandle_t	wireframeAutomapFrame_left;
	qhandle_t	wireframeAutomapFrame_right;
	qhandle_t	wireframeAutomapFrame_top;
	qhandle_t	wireframeAutomapFrame_bottom;

//JAPRO - Clientside - Movement keys - Start
	qhandle_t	keyCrouchOffShader;
	qhandle_t	keyCrouchOnShader;
	qhandle_t	keyJumpOffShader;
	qhandle_t	keyJumpOnShader;
	qhandle_t	keyBackOffShader;
	qhandle_t	keyBackOnShader;
	qhandle_t	keyForwardOffShader;
	qhandle_t	keyForwardOnShader;
	qhandle_t	keyLeftOffShader;
	qhandle_t	keyLeftOnShader;
	qhandle_t	keyRightOffShader;
	qhandle_t	keyRightOnShader;
//JAPRO - Clientside - Movement keys - End

	qhandle_t	bloodExplosionShader;//JAPRO - Clientside - Re add cg_blood
	qhandle_t	leadIndicator;//JAPRO - Clientside - Re add cg_blood

	qhandle_t	grappleModel;//grapple model

//Chunks
	qhandle_t	chunkModels[NUM_CHUNK_TYPES][4];
	sfxHandle_t	chunkSound;
	sfxHandle_t	grateSound;
	sfxHandle_t	rockBreakSound;
	sfxHandle_t	rockBounceSound[2];
	sfxHandle_t	metalBounceSound[2];
	sfxHandle_t	glassChunkSound;
	sfxHandle_t	crateBreakSound[2];

	qhandle_t	hackerIconShader;

	// Saber shaders
	//-----------------------------
	qhandle_t	forceCoronaShader;

	qhandle_t	redSaberGlowShader;
	qhandle_t	redSaberCoreShader;
	qhandle_t	orangeSaberGlowShader;
	qhandle_t	orangeSaberCoreShader;
	qhandle_t	yellowSaberGlowShader;
	qhandle_t	yellowSaberCoreShader;
	qhandle_t	greenSaberGlowShader;
	qhandle_t	greenSaberCoreShader;
	qhandle_t	blueSaberGlowShader;
	qhandle_t	blueSaberCoreShader;
	qhandle_t	purpleSaberGlowShader;
	qhandle_t	purpleSaberCoreShader;

	//rgb
	qhandle_t	rgbSaberGlowShader;
	qhandle_t	rgbSaberCoreShader;

	//sfx
	qhandle_t	ShaderSaberTrail;
	qhandle_t	ShaderSaberBlade;
	qhandle_t	ShaderSaberBladeRGB;
	qhandle_t	ShaderSaberEnd;
	qhandle_t	ShaderSaberEndRGB;

	//rp
	qhandle_t saberIgnitionFlare;
	//qhandle_t blackIgnitionFlare;

#define _SHITTYLINEFX 1
#if _SHITTYLINEFX
	qhandle_t	rgbSaberGlow2Shader;
	qhandle_t	rgbSaberCore2Shader;
	qhandle_t	rgbSaberTrail2Shader;

	qhandle_t	rgbSaberGlow3Shader;
	qhandle_t	rgbSaberCore3Shader;
	qhandle_t	rgbSaberTrail3Shader;

	qhandle_t	rgbSaberGlow4Shader;
	qhandle_t	rgbSaberCore4Shader;
	qhandle_t	rgbSaberTrail4Shader;

	qhandle_t	rgbSaberGlow5Shader;
	qhandle_t	rgbSaberCore5Shader;
	qhandle_t	rgbSaberTrail5Shader;
#endif

#if _GRAPPLE
	qhandle_t	grappleShader;
#endif

	qhandle_t	blackSaberGlowShader;
	qhandle_t	blackSaberCoreShader;
	qhandle_t	blackBlurShader;
	//rgb

	qhandle_t	saberBlurShader;
	qhandle_t	swordTrailShader;

	qhandle_t	yellowDroppedSaberShader;

	qhandle_t	rivetMarkShader;

	qhandle_t	saberClashFlare;

	//JAPRO - Clientside - Use all saber hum sounds found in base assets
	qhandle_t	saberHumSounds[5];

	qhandle_t	teamRedShader;
	qhandle_t	teamBlueShader;

	qhandle_t	powerDuelAllyShader;

	qhandle_t	balloonShader;
	qhandle_t	vchatShader;
	qhandle_t	connectionShader;

	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;

	qhandle_t	numberShaders[11];
	qhandle_t	smallnumberShaders[11];
	qhandle_t	chunkyNumberShaders[11];

	qhandle_t	electricBodyShader;
	qhandle_t	electricBody2Shader;

	qhandle_t	fsrMarkShader;
	qhandle_t	fslMarkShader;
	qhandle_t	fshrMarkShader;
	qhandle_t	fshlMarkShader;

	qhandle_t	refractionShader;

	qhandle_t	cloakedShader;

	qhandle_t	boltShader;

	qhandle_t	shadowMarkShader;

	//glass shard shader
	qhandle_t	glassShardShader;

	// wall mark shaders
	qhandle_t	wakeMarkShader;

	// Pain view shader
	qhandle_t	viewPainShader;
	qhandle_t	viewPainShader_Shields;
	qhandle_t	viewPainShader_ShieldsAndHealth;

	qhandle_t	itemRespawningPlaceholder;
	qhandle_t	itemRespawningRezOut;

	qhandle_t	playerShieldDamage;
	qhandle_t	protectShader;
	qhandle_t	forceSightBubble;
	qhandle_t	forceShell;
	qhandle_t	sightShell;

	// Disruptor zoom graphics
	qhandle_t	disruptorMask;
	qhandle_t	disruptorInsert;
	qhandle_t	disruptorLight;
	qhandle_t	disruptorInsertTick;
	qhandle_t	disruptorChargeShader;

	// Binocular graphics
	qhandle_t	binocularCircle;
	qhandle_t	binocularMask;
	qhandle_t	binocularArrow;
	qhandle_t	binocularTri;
	qhandle_t	binocularStatic;
	qhandle_t	binocularOverlay;

	// weapon effect models
	qhandle_t	lightningExplosionModel;

	// explosion assets
	qhandle_t	explosionModel;
	qhandle_t	surfaceExplosionShader;

	qhandle_t	disruptorShader;

	qhandle_t	solidWhite;

	qhandle_t	heartShader;

	// All the player shells
	qhandle_t	ysaliredShader;
	qhandle_t	ysaliblueShader;
	qhandle_t	ysalimariShader;
	qhandle_t	boonShader;
	qhandle_t	endarkenmentShader;
	qhandle_t	enlightenmentShader;
	qhandle_t	invulnerabilityShader;

#define JK2AWARDS
#ifdef JK2AWARDS
	// medals shown during gameplay
	qhandle_t	medalImpressive;
	qhandle_t	medalExcellent;
	qhandle_t	medalGauntlet;
	qhandle_t	medalDefend;
	qhandle_t	medalAssist;
	qhandle_t	medalCapture;

	qhandle_t	medalImpressiveQ3;
	qhandle_t	medalExcellentQ3;
	qhandle_t	medalGauntletQ3;
	qhandle_t	medalDefendQ3;
	qhandle_t	medalAssistQ3;
	qhandle_t	medalCaptureQ3;
#endif

	//JK2HUD
	qhandle_t	JK2HUDLeftFrame;
	qhandle_t	JK2HUDInnerLeft;
	qhandle_t	JK2HUDArmor1;
	qhandle_t	JK2HUDArmor2;
	qhandle_t	JK2HUDHealth;
	qhandle_t	JK2HUDHealthTic;
	qhandle_t	JK2HUDArmorTic;
	qhandle_t	JK2HUDSaberStyle1;
	qhandle_t	JK2HUDSaberStyle2;
	qhandle_t	JK2HUDSaberStyle3;
	qhandle_t	JK2HUDSaberStyle4;
	qhandle_t	JK2HUDRightFrame;
	qhandle_t	JK2HUDInnerRight;
	qhandle_t	JK2weaponProngsOn;
	qhandle_t	JK2weaponProngsOff;
	qhandle_t	JK2forceProngsOn;
	qhandle_t	JK2inventoryProngsOn;

	//unused weapon icons for staff/duals
	qhandle_t	weaponIconsStaff;
	qhandle_t	weaponIconsAkimbo;

	// sounds
	sfxHandle_t	selectSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];

	sfxHandle_t	winnerSound;
	sfxHandle_t	loserSound;

	sfxHandle_t crackleSound;

	sfxHandle_t	grenadeBounce1;
	sfxHandle_t	grenadeBounce2;

	sfxHandle_t teamHealSound;
	sfxHandle_t teamRegenSound;

	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
	sfxHandle_t	respawnSound;
	sfxHandle_t talkSound;
	sfxHandle_t	teamChatSound;
	sfxHandle_t	privateChatSound;
	sfxHandle_t landSound;
	sfxHandle_t fallSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;

//JAPRO - Clientside - Hitsounds Start
	sfxHandle_t hitSound;
	sfxHandle_t hitSound2;
	sfxHandle_t hitSound3;
	sfxHandle_t hitSound4;
	sfxHandle_t hitTeamSound;
//JAPRO - Clientside - Hitsounds End

#ifdef JK2AWARDS
	sfxHandle_t firstImpressiveSound;
	sfxHandle_t impressiveSound;
	sfxHandle_t firstExcellentSound;
	sfxHandle_t excellentSound;
	sfxHandle_t firstHumiliationSound;
	sfxHandle_t humiliationSound;
	sfxHandle_t deniedSound;
	sfxHandle_t defendSound;
	sfxHandle_t assistSound;

	sfxHandle_t firstImpressiveSoundQ3;
	sfxHandle_t impressiveSoundQ3;
	sfxHandle_t firstExcellentSoundQ3;
	sfxHandle_t excellentSoundQ3;
	sfxHandle_t deniedSoundQ3;
	sfxHandle_t firstHumiliationSoundQ3;
	sfxHandle_t humiliationSoundQ3;
#endif

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t rollSound;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t noforceSound;

	sfxHandle_t deploySeeker;
	sfxHandle_t medkitSound;

	// teamplay sounds
#ifdef JK2AWARDS
	sfxHandle_t captureAwardSound;
#endif
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	sfxHandle_t redFlagReturnedSound;
	sfxHandle_t blueFlagReturnedSound;
	sfxHandle_t	redTookFlagSound;
	sfxHandle_t blueTookFlagSound;

	sfxHandle_t redYsalReturnedSound;
	sfxHandle_t blueYsalReturnedSound;
	sfxHandle_t	redTookYsalSound;
	sfxHandle_t blueTookYsalSound;

	sfxHandle_t	drainSound;

	//music blips
	sfxHandle_t	happyMusic;
	sfxHandle_t dramaticFailure;

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;

	// new stuff
	qhandle_t patrolShader;
	qhandle_t assaultShader;
	qhandle_t campShader;
	qhandle_t followShader;
	qhandle_t defendShader;
	qhandle_t retrieveShader;
	qhandle_t escortShader;

	qhandle_t halfShieldModel;
	qhandle_t halfShieldShader;

	qhandle_t demp2Shell;
	qhandle_t demp2ShellShader;

	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;

	//weapon icons
	qhandle_t weaponIcons[WP_NUM_WEAPONS];
	qhandle_t weaponIcons_NA[WP_NUM_WEAPONS];

	//holdable inventory item icons
	qhandle_t invenIcons[HI_NUM_HOLDABLE];

	//force power icons
	qhandle_t forcePowerIcons[NUM_FORCE_POWERS];

	qhandle_t rageRecShader;

	//other HUD parts
	int			currentBackground;
	qhandle_t	weaponIconBackground;
	qhandle_t	forceIconBackground;
	qhandle_t	inventoryIconBackground;

	sfxHandle_t	holocronPickup;

	// Zoom
	sfxHandle_t	zoomStart;
	sfxHandle_t	zoomLoop;
	sfxHandle_t	zoomEnd;
	sfxHandle_t	disruptorZoomLoop;

	qhandle_t	bdecal_bodyburn1;
	qhandle_t	bdecal_saberglow;
	qhandle_t	bdecal_burn1;
	qhandle_t	mSaberDamageGlow;

	// For vehicles only now
	sfxHandle_t	noAmmoSound;

	qhandle_t	lightningShader; // japro loda

	//japro gibs
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

	qhandle_t	gibSound;
	qhandle_t	gibBounce1Sound;
	qhandle_t	gibBounce2Sound;
	qhandle_t	gibBounce3Sound;
	qhandle_t	bloodTrailShader;
	qhandle_t	bloodMarkShader;

	qhandle_t	raceShader;
	qhandle_t	duelShader;

	qhandle_t	tripMineModel;

	//japro cosmetics
	struct {
		qhandle_t	santaHat;
		qhandle_t	pumpkin;
		qhandle_t	cap;
		qhandle_t	fedora;
		qhandle_t	kringekap;
		qhandle_t	sombrero;
		qhandle_t	tophat;
		//qhandle_t	fire;
	} cosmetics;

	//japro vgs
	sfxHandle_t		maleVGSSounds[MAX_CUSTOM_VGS_SOUNDS];
	sfxHandle_t		femaleVGSSounds[MAX_CUSTOM_VGS_SOUNDS];

} cgMedia_t;


// Stored FX handles
//--------------------
typedef struct cgEffects_s {
	//concussion
	fxHandle_t	concussionShotEffect;
	fxHandle_t	concussionImpactEffect;

	// BRYAR PISTOL
	fxHandle_t	bryarShotEffect;
	fxHandle_t	bryarPowerupShotEffect;
	fxHandle_t	bryarWallImpactEffect;
	fxHandle_t	bryarWallImpactEffect2;
	fxHandle_t	bryarWallImpactEffect3;
	fxHandle_t	bryarFleshImpactEffect;
	fxHandle_t	bryarDroidImpactEffect;

	// BLASTER
	fxHandle_t  blasterShotEffect;
	fxHandle_t  blasterWallImpactEffect;
	fxHandle_t  blasterFleshImpactEffect;
	fxHandle_t  blasterDroidImpactEffect;

	// DISRUPTOR
	fxHandle_t  disruptorRingsEffect;
	fxHandle_t  disruptorProjectileEffect;
	fxHandle_t  disruptorWallImpactEffect;
	fxHandle_t  disruptorFleshImpactEffect;
	fxHandle_t  disruptorAltMissEffect;
	fxHandle_t  disruptorAltHitEffect;

	// BOWCASTER
	fxHandle_t	bowcasterShotEffect;
	fxHandle_t	bowcasterImpactEffect;

	// REPEATER
	fxHandle_t  repeaterProjectileEffect;
	fxHandle_t  repeaterAltProjectileEffect;
	fxHandle_t  repeaterWallImpactEffect;
	fxHandle_t  repeaterFleshImpactEffect;
	fxHandle_t  repeaterAltWallImpactEffect;

	// DEMP2
	fxHandle_t  demp2ProjectileEffect;
	fxHandle_t  demp2WallImpactEffect;
	fxHandle_t  demp2FleshImpactEffect;

	// FLECHETTE
	fxHandle_t	flechetteShotEffect;
	fxHandle_t	flechetteAltShotEffect;
	fxHandle_t	flechetteWallImpactEffect;
	fxHandle_t	flechetteFleshImpactEffect;

	// ROCKET
	fxHandle_t  rocketShotEffect;
	fxHandle_t  rocketExplosionEffect;

	// THERMAL
	fxHandle_t	thermalExplosionEffect;
	fxHandle_t	thermalShockwaveEffect;

	// TRIPMINE
	fxHandle_t	tripmineLaserFX;
	fxHandle_t	tripmineGlowFX;

	//FORCE
	fxHandle_t forceLightning;
	fxHandle_t forceLightningWide;

	fxHandle_t forceDrain;
	fxHandle_t forceDrainWide;
	fxHandle_t forceDrainWideJaPRO;//japro
	fxHandle_t forceDrained;

	fxHandle_t heal2FX;

	//TURRET
	fxHandle_t turretShotEffect;

	//Whatever
	fxHandle_t itemCone;

	fxHandle_t	mSparks;
	fxHandle_t	mSaberCut;
	fxHandle_t	mTurretMuzzleFlash;
	fxHandle_t	mSaberBlock;
	fxHandle_t	mSaberBloodSparks;
	fxHandle_t	mSaberBloodSparksSmall;
	fxHandle_t	mSaberBloodSparksMid;
	fxHandle_t	mSpawn;
	fxHandle_t	mJediSpawn;
	fxHandle_t	mBlasterDeflect;
	fxHandle_t	mBlasterSmoke;
	fxHandle_t	mForceConfustionOld;
	fxHandle_t	mDisruptorDeathSmoke;
	fxHandle_t	mSparkExplosion;
	fxHandle_t	mTurretExplode;
	fxHandle_t	mEmplacedExplode;
	fxHandle_t	mEmplacedDeadSmoke;
	fxHandle_t	mTripmineExplosion;
	fxHandle_t	mDetpackExplosion;
	fxHandle_t	mFlechetteAltBlow;
	fxHandle_t	mStunBatonFleshImpact;
	fxHandle_t	mAltDetonate;
	fxHandle_t	mSparksExplodeNoSound;
	fxHandle_t	mTripMineLaser;
	fxHandle_t	mEmplacedMuzzleFlash;
	fxHandle_t	mConcussionAltRing;
	fxHandle_t	mHyperspaceStars;
	fxHandle_t	mBlackSmoke;
	fxHandle_t	mShipDestDestroyed;
	fxHandle_t	mShipDestBurning;
	fxHandle_t	mBobaJet;

	//footstep effects
	fxHandle_t footstepMud;
	fxHandle_t footstepSand;
	fxHandle_t footstepSnow;
	fxHandle_t footstepGravel;
	//landing effects
	fxHandle_t landingMud;
	fxHandle_t landingSand;
	fxHandle_t landingDirt;
	fxHandle_t landingSnow;
	fxHandle_t landingGravel;
	//splashes
	fxHandle_t waterSplash;
	fxHandle_t lavaSplash;
	fxHandle_t acidSplash;

#if _GRAPPLE
	fxHandle_t grappleHitWall;
	fxHandle_t grappleHitPlayer;
#endif

	//breath effects
	fxHandle_t	breath;
	fxHandle_t	waterBreath;
} cgEffects_t;

#define MAX_STATIC_MODELS 4000

typedef struct cg_staticmodel_s {
	qhandle_t		model;
	vec3_t			org;
	matrix3_t		axes;
	float			radius;
	float			zoffset;
} cg_staticmodel_t;

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournament restart is done, allowing
// all clients to begin playing instantly
typedef struct cgs_s {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	int				siegeTeamSwitch;
	int				showDuelHealths;
	gametype_t		gametype;
	int				debugMelee;
	int				stepSlideFix;
	int				noSpecMove;
	int				dmflags;
	int				fraglimit;
	int				duel_fraglimit;
	int				capturelimit;
	int				timelimit;
	int				maxclients;
	qboolean		needpass;
	qboolean		jediVmerc;
	int				wDisable;
	int				fDisable;

	char			mapname[MAX_QPATH];
	char			rawmapname[MAX_QPATH];
//	char			redTeam[MAX_QPATH];
//	char			blueTeam[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				teamVoteTime[2];
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	qboolean		teamVoteModified[2];	// beep whenever changed
	char			teamVoteString[2][MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				jediMaster;
	int				duelWinner;
	int				duelist1;
	int				duelist2;
	int				duelist3;
// nmckenzie: DUEL_HEALTH.  hmm.
	int				duelist1health;
	int				duelist2health;
	int				duelist3health;

	int				redflag, blueflag;		// flag status from configstrings
	int				flagStatus;

	//new flagstatus stuff
	clientInfo_t	*redFlagCarrier, *blueFlagCarrier;
	int				redFlagTime, blueFlagTime;

//[JAPRO - Clientside - All - Add cinfo variables to get cinfo from server japlus and japro servers - Start]
	serverMod_t	serverMod;
	int			cinfo;
	int			jcinfo;
	int			jcinfo2;
	qboolean	pluginSet;
	qboolean	legacyProtocol; //for compatibility with 1.00 servers
	int			restricts;//make this a short?

	int			svfps;
	qboolean	takenscreenshot;
	int			hookpull;
	unsigned int ignoredVGS;
//[JAPRO - Clientside - All - Add cinfo variables to get cinfo from server japlus and japro servers - End]

	qboolean	cvarsRegistered;
	qboolean	newHud, hudLoaded;
	qboolean	jaPROEngine; //egh
	float		widthRatioCoef;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];
	fxHandle_t		gameEffects[MAX_FX];
	qhandle_t		gameIcons[MAX_ICONS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	float cursorX;
	float cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void *capturedItem;
	qhandle_t activeCursor;

	// media
	cgMedia_t		media;

	// effects
	cgEffects_t		effects;

	int					numMiscStaticModels;
	cg_staticmodel_t	miscStaticModels[MAX_STATIC_MODELS];

} cgs_t;

typedef struct siegeExtended_s
{
	int			health;
	int			maxhealth;
	int			ammo;
	int			weapon;
	int			lastUpdated;
} siegeExtended_t;

//keep an entry available for each client
extern siegeExtended_t cg_siegeExtendedData[MAX_CLIENTS];

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];

extern	centity_t		*cg_permanents[MAX_GENTITIES];
extern	int				cg_numpermanents;

extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];

extern qboolean			cg_legacyCGameAPI;

// cg_cvar.c
#define XCVAR_PROTO
	#include "cg_xcvar.h"
#undef XCVAR_PROTO
void CG_RegisterCvars( void );
void CG_UpdateCvars( void );

//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void CG_StartMusic( qboolean bForceStart );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );
int CG_LastWhisperer( void );
void CG_LoadMenus(const char *menuFile);
void CG_LogPrintf(fileHandle_t fileHandle, const char *fmt, ...); //chatlog
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
void CG_SetScoreSelection(void *menu);
void CG_BuildSpectatorString(void);
void CG_NextInventory_f(void);
void CG_PrevInventory_f(void);
void CG_NextForcePower_f(void);
void CG_PrevForcePower_f(void);

//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_AddBufferedSound( sfxHandle_t sfx);

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
/*
Ghoul2 Insert Start
*/

void CG_TestG2Model_f (void);
void CG_TestModelSurfaceOnOff_f(void);
void CG_ListModelSurfaces_f (void);
void CG_ListModelBones_f (void);
void CG_TestModelSetAnglespre_f(void);
void CG_TestModelSetAnglespost_f(void);
void CG_TestModelAnimate_f(void);
/*
Ghoul2 Insert End
*/

//
// cg_drawtools.c
//
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawRotatePic( float x, float y, float width, float height,float angle, qhandle_t hShader );
void CG_DrawRotatePic2( float x, float y, float width, float height,float angle, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string,
				   float charWidth, float charHeight, const float *modulate );

void CG_DrawNumField (float x, float y, int width, int value, float charWidth, float charHeight, int style, qboolean zeroFill);

void CG_DrawStringExt( float x, float y, const char *string, const float *setColor,
		qboolean forceColor, qboolean shadow, float charWidth, float charHeight, int maxChars );
void CG_DrawBigString( float x, float y, const char *s, float alpha );
void CG_DrawBigStringColor( float x, float y, const char *s, vec4_t color );
void CG_DrawSmallString( float x, float y, const char *s, float alpha );
void CG_DrawSmallStringColor( float x, float y, const char *s, vec4_t color );

int CG_DrawStrlen( const char *str );

#if NEW_SCOREBOARD
void CG_LerpColour( const vec4_t start, const vec4_t end, vec4_t out, float point );
#endif

float	*CG_FadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void CG_DrawProportionalString(float x, float y, const char* str, int style, vec4_t color);
void CG_DrawScaledProportionalString(float x, float y, const char* str, int style, vec4_t color, float scale);
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);

//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern  char systemChat[256];

void CG_AddLagometerFrameInfo( void );
void CG_AddSpeedGraphFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_CenterPrintMultiKill(const char *str, int y, int charWidth);
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle,int font);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, int iMenuFont);
float CG_Text_Width(const char *text, float scale, int iMenuFont);
int CG_Text_WidthInt(const char *text, float scale, int iMenuFont);
int CG_Text_Height(const char *text, float scale, int iMenuFont);
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
qboolean CG_DeferMenuScript(char **args);
void CG_ShowResponseHead(void);
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText(void);
const char *CG_GetKillerText(void);
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, void *ghoul2, int g2radius, qhandle_t skin, vec3_t origin, vec3_t angles );
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
qboolean CG_YourTeamHasFlag(void);
qboolean CG_OtherTeamHasFlag(void);
qhandle_t CG_StatusHandle(int task);

//japro
qboolean CG_WorldCoordToScreenCoord(vec3_t worldCoord, float *x, float *y);



//
// cg_player.c
//
qboolean CG_RagDoll(centity_t *cent, vec3_t forcedAngles);
qboolean CG_G2TraceCollide(trace_t *tr, const vec3_t mins, const vec3_t maxs, const vec3_t lastValidStart, const vec3_t lastValidEnd);
void CG_AddGhoul2Mark(int shader, float size, vec3_t start, vec3_t end, int entnum,
					  vec3_t entposition, float entangle, void *ghoul2, vec3_t scale, int lifeTime);

void CG_CreateNPCClient(clientInfo_t **ci);
void CG_DestroyNPCClient(clientInfo_t **ci);

void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team );
void CG_NewClientInfo( int clientNum, qboolean entitiesInitialized );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );
void CG_PlayerShieldHit(int entitynum, vec3_t angles, int amount);


//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
void CG_G2Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
void CG_CrosshairTrace(trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, qboolean g2Check); //japro
void CG_PredictPlayerState( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );
void CG_ReattachLimb(centity_t *source);


//
// cg_ents.c
//

void CG_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void CG_S_AddRealLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void CG_S_StopLoopingSound(int entityNum, sfxHandle_t sfx);
void CG_S_UpdateLoopingSounds(int entityNum);

void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( qboolean isPortal );
void CG_ManualEntityRender(centity_t *cent);
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );

qboolean CG_DuelCull( centity_t *cent );

/*
Ghoul2 Insert Start
*/
void ScaleModelAxis(refEntity_t	*ent);
/*
Ghoul2 Insert End
*/

//
// cg_turret.c
//
void TurretClientRun(centity_t *ent);

//
// cg_weapons.c
//
void CG_GetClientWeaponMuzzleBoltPoint(int clIndex, vec3_t to);

void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );
void CG_WeaponClean_f( void );

void CG_RegisterWeapon( int weaponNum);
void CG_RegisterItemVisuals( int itemNum );

void CG_FireWeapon( centity_t *cent, qboolean alt_fire );
void CG_MissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType, qboolean alt_fire, int charge);
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum, qboolean alt_fire);

void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team, vec3_t newAngles, qboolean thirdPerson );
void CG_DrawWeaponSelect( void );
void CG_DrawIconBackground(void);

void CG_OutOfAmmoChange( int oldWeapon );	// should this be in pmove?

//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader,
				    const vec3_t origin, const vec3_t dir,
					float orientation,
				    float r, float g, float b, float a,
					qboolean alphaFade,
					float radius, qboolean temporary );

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );
#if _NEWTRAILS
void CG_InitStrafeTrails( void );
strafeTrail_t	*CG_AllocStrafeTrail( void );
#endif

#define MAX_COSMETIC_UNLOCKS 32
typedef struct CosmeticUnlocks_s {
	unsigned short	bitvalue;
	char			mapname[40];
	short			style;
	unsigned int	duration;
	qboolean		active;
} CosmeticUnlocks_t;
CosmeticUnlocks_t cosmeticUnlocks[MAX_COSMETIC_UNLOCKS];
//japro

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p,
				   const vec3_t vel,
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader );
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
void CG_GlassShatter(int entnum, vec3_t dmgPt, vec3_t dmgDir, float dmgRadius, int maxShards);
void CG_ScorePlum( int client, vec3_t org, int score );

void CG_Chunks( int owner, vec3_t origin, const vec3_t normal, const vec3_t mins, const vec3_t maxs,
						float speed, int numChunks, material_t chunkType, int customChunk, float baseScale );
void CG_MiscModelExplosion( vec3_t mins, vec3_t maxs, int size, material_t chunkType );

void CG_Bleed( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
								qhandle_t hModel, int numframes, qhandle_t shader, int msec,
								qboolean isSprite, float scale, int flags );// Overloaded in single player

void CG_SurfaceExplosion( vec3_t origin, vec3_t normal, float radius, float shake_speed, qboolean smoke );

void CG_TestLine( vec3_t start, vec3_t end, int time, unsigned int color, float radius);
//void CG_TestLine( vec3_t start, vec3_t end, int time, unsigned int color, float radius, qhandle_t customShader );

void CG_InitGlass( void );

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );
//unlagged - early transitioning
void CG_TransitionEntity( centity_t *cent );
//unlagged - early transitioning

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingItem( int itemNum );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );

//
// cg_spawn.c
//
qboolean	CG_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean	CG_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean	CG_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean	CG_SpawnBoolean( const char *key, const char *defaultString, qboolean *out );
qboolean	CG_SpawnVector( const char *key, const char *defaultString, float *out );
void		CG_ParseEntitiesFromString( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );
void CG_DrawOldTourneyScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );
void CG_ShaderStateChanged(void);

//
// cg_playerstate.c
//
int CG_IsMindTricked(int trickIndex1, int trickIndex2, int trickIndex3, int trickIndex4, int client);
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );


//
// cg_siege.c
//
void CG_InitSiegeMode(void);
void CG_SiegeRoundOver(centity_t *ent, int won);
void CG_SiegeObjectiveCompleted(centity_t *ent, int won, int objectivenum);



//===============================================

void		BG_CycleInven(playerState_t *ps, int direction);
int			BG_ProperForceIndex(int power);
void		BG_CycleForce(playerState_t *ps, int direction);

const char *CG_GetStringEdString(char *refSection, char *refName);

void FX_TurretProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon );
void FX_TurretHitWall( vec3_t origin, vec3_t normal );
void FX_TurretHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid );

void FX_ConcussionHitWall( vec3_t origin, vec3_t normal );
void FX_ConcussionHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid );
void FX_ConcussionProjectileThink(  centity_t *cent, const struct weaponInfo_s *weapon );
void FX_ConcAltShot( vec3_t start, vec3_t end );

//-----------------------------
// Effects related prototypes
//-----------------------------

// Environmental effects
void CG_Spark( vec3_t origin, vec3_t dir );

// Weapon prototypes
void FX_BryarHitWall( vec3_t origin, vec3_t normal );
void FX_BryarAltHitWall( vec3_t origin, vec3_t normal, int power );
void FX_BryarHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid );
void FX_BryarAltHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid );

void FX_BlasterProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BlasterAltFireThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BlasterWeaponHitWall( vec3_t origin, vec3_t normal );
void FX_BlasterWeaponHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid );


void FX_ForceDrained(vec3_t origin, vec3_t dir);


//-----------------------------
// Effects related prototypes
//-----------------------------

// Environmental effects
void CG_Spark( vec3_t origin, vec3_t dir );

// Weapon prototypes
void FX_BryarHitWall( vec3_t origin, vec3_t normal );
void FX_BryarAltHitWall( vec3_t origin, vec3_t normal, int power );
void FX_BryarHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid );
void FX_BryarAltHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid );

void FX_BlasterProjectileThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BlasterAltFireThink( centity_t *cent, const struct weaponInfo_s *weapon );
void FX_BlasterWeaponHitWall( vec3_t origin, vec3_t normal );
void FX_BlasterWeaponHitPlayer( vec3_t origin, vec3_t normal, qboolean humanoid );

void		CG_Init_CG(void);
void		CG_Init_CGents(void);


void CG_SetGhoul2Info( refEntity_t *ent, centity_t *cent);
void CG_CreateBBRefEnts(entityState_t *s1, vec3_t origin );

void CG_InitG2Weapons(void);
void CG_ShutDownG2Weapons(void);
void CG_CopyG2WeaponInstance(centity_t *cent, int weaponNum, void *toGhoul2);
void *CG_G2WeaponInstance(centity_t *cent, int weapon);
void CG_CheckPlayerG2Weapons(playerState_t *ps, centity_t *cent);

void CG_SetSiegeTimerCvar( int msec );

void	CG_ClearLightStyles (void);
void	CG_RunLightStyles (void);
void	CG_SetLightstyle (int i);

/*
Ghoul2 Insert End
*/

extern cgameImport_t *trap;
