#include "Renderer/Renderer_PCH.h"
#pragma hdrstop
#include <Renderer/Animation.h>

mxDEFINE_CLASS(rxTranslationTrack);
mxBEGIN_REFLECTION(rxTranslationTrack)
mxMEMBER_FIELD(times),
mxMEMBER_FIELD(values),
mxEND_REFLECTION

mxDEFINE_CLASS(rxRotationTrack);
mxBEGIN_REFLECTION(rxRotationTrack)
mxMEMBER_FIELD(times),
mxMEMBER_FIELD(values),
mxEND_REFLECTION

mxDEFINE_CLASS(rxAnimTrack);
mxBEGIN_REFLECTION(rxAnimTrack)
mxMEMBER_FIELD(translations),
mxMEMBER_FIELD(rotations),
mxEND_REFLECTION
rxAnimTrack::rxAnimTrack()
{
}

mxDEFINE_CLASS(rxAnimClip);
mxBEGIN_REFLECTION(rxAnimClip)
mxMEMBER_FIELD(tracks),
mxMEMBER_FIELD(nodes),
mxMEMBER_FIELD(name),
mxMEMBER_FIELD(length),
mxEND_REFLECTION;
rxAnimClip::rxAnimClip()
{
	length = 0.0f;
}

const rxAnimTrack* rxAnimClip::FindTrackByName( const char* name ) const
{
	for( int i = 0; i < tracks.Num(); i++ ) {
		if(Str::EqualS(nodes[i], name)) {
			return &tracks[i];
		}
	}
	return nil;
}

mxDEFINE_CLASS(rxAnimSet);
mxBEGIN_REFLECTION(rxAnimSet)
mxMEMBER_FIELD(animations),
mxMEMBER_FIELD(name),
mxEND_REFLECTION;
rxAnimSet::rxAnimSet()
{
}
const rxAnimClip* rxAnimSet::FindAnimByName(const char* name) const
{
	for( UINT animIndex = 0; animIndex < animations.Num(); animIndex++ ) {
		if(Str::EqualS(animations[animIndex].name, name)) {
			return &animations[animIndex];
		}
	}
	return nil;
}

mxDEFINE_CLASS(rxAnimInstance);
mxBEGIN_REFLECTION(rxAnimInstance)
	mxMEMBER_FIELD(target),
mxEND_REFLECTION
rxAnimInstance::rxAnimInstance()
{
	target = nil;
}

//--------------------------------------------------------------//
//				End Of File.									//
//--------------------------------------------------------------//
