// Animation
#pragma once

#include <Graphics/Device.h>
#include <Graphics/Geometry.h>

struct rxTranslationTrack : public CStruct
{
	TBuffer< float >	times;	// key frame timestamps
	TBuffer< Float3 >	values;	// translation vectors
public:
	mxDECLARE_CLASS(rxTranslationTrack,CStruct);
	mxDECLARE_REFLECTION;
};
struct rxRotationTrack : public CStruct
{
	TBuffer< float >	times;	// key frame timestamps
	TBuffer< Vector4 >	values;	// rotation quaternions
public:
	mxDECLARE_CLASS(rxRotationTrack,CStruct);
	mxDECLARE_REFLECTION;
};

// AnimTrack/BoneTrack/AnimCurve
// There is a single track for each animated property of each element.
// Frames between keyframes are linearly interpolated.
// NOTE: all bones use the same number of key frames, at the same times.
struct rxAnimTrack : public CStruct
{
	rxTranslationTrack	translations;
	rxRotationTrack		rotations;
public:
	mxDECLARE_CLASS(rxAnimTrack,CStruct);
	mxDECLARE_REFLECTION;
	rxAnimTrack();
};

// Animation/AnimClip/AnimSequence
// A basic key-framed animation - a collection of related animation curves.
// An animation clip contains a set of tracks.
struct rxAnimClip : public CStruct
{
	// The 'tracks' array has the same number of entries as 'nodes'.
	TBuffer< rxAnimTrack >	tracks;	// bone tracks - raw animation data
	TBuffer< String >		nodes;	// names of the animated component (e.g. a bone or node)
	String			name;
	float		length;	// duration of the animation in seconds
public:
	mxDECLARE_CLASS(rxAnimClip,CStruct);
	mxDECLARE_REFLECTION;
	rxAnimClip();
	const rxAnimTrack* FindTrackByName( const char* name ) const;
};

// AnimSet is a collection of related animation clips
// (for instance, all animation clips of a character).
struct rxAnimSet : public CStruct
{
	TBuffer< rxAnimClip >	animations;	// aka 'animations', 'sequences'
	String					name;		// e.g. 'NPC_Soldier_Anims'
public:
	mxDECLARE_CLASS(rxAnimSet,CStruct);
	mxDECLARE_REFLECTION;
	rxAnimSet();
	const rxAnimClip* FindAnimByName(const char* name) const;
};

// AnimInstance/AnimController
// AnimInstance is used for animating a skeleton instance (Skeleton).
class rxAnimInstance : public CStruct
{
public:
	Skeleton *	target;

public:
	mxDECLARE_CLASS(rxAnimInstance,CStruct);
	mxDECLARE_REFLECTION;
	rxAnimInstance();
};

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
