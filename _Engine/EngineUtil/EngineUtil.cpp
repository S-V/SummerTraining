#include <Base/Base.h>
#pragma hdrstop
#include <EngineUtil/EngineUtil.h>

/*
-----------------------------------------------------------------------------
	OrbitingCamera
-----------------------------------------------------------------------------
*/
mxDEFINE_CLASS(OrbitingCamera);
mxBEGIN_REFLECTION(OrbitingCamera)
	mxMEMBER_FIELD(arcBall),
	mxMEMBER_FIELD(radius),
mxEND_REFLECTION
OrbitingCamera::OrbitingCamera()
{
	radius = 1.0f;
}
Float4x4 OrbitingCamera::GetTransform() const
{
	// offset is applied in world space
	Float4x4 offsetMatrix = Matrix_Translation(0.0f, 0.0f, -radius);
	return Matrix_Multiply(arcBall.GetRotationMatrix(), offsetMatrix);
}
void OrbitingCamera::OnInputEvent( const SInputEvent& _event )
{
	UNDONE;
#if 0
	if( _event.type == Event_MouseButtonEvent )
	{
		if( _event.mouseButton.action == IA_Pressed )
		{
			arcBall.BeginDrag(_event.mouse.x, _event.mouse.y);
		}
		if( _event.mouseButton.action == IA_Released )
		{
			arcBall.EndDrag();
		}
	}
	if( _event.type == Event_MouseCursorMoved )
	{
		arcBall.MoveDrag(_event.mouse.x, _event.mouse.y);
	}
#endif
}



mxDEFINE_CLASS(FlyingCamera);
mxBEGIN_REFLECTION(FlyingCamera)
	mxMEMBER_FIELD(m_eyePosition),
	mxMEMBER_FIELD(m_upDirection),
	mxMEMBER_FIELD(m_lookDirection),
	mxMEMBER_FIELD(m_rightDirection),
	
	mxMEMBER_FIELD(m_linearVelocity),
	mxMEMBER_FIELD(m_linearFriction),
	mxMEMBER_FIELD(m_angularFriction),

	mxMEMBER_FIELD(m_movementSpeed),
	mxMEMBER_FIELD(m_strafingSpeed),
	mxMEMBER_FIELD(m_rotationSpeed),

	mxMEMBER_FIELD(m_hasLinearMomentum),
	mxMEMBER_FIELD(m_hasAngularMomentum),
	mxMEMBER_FIELD(m_invertPitch),
	mxMEMBER_FIELD(m_invertYaw),

	mxMEMBER_FIELD(m_pitchVelocity),
	mxMEMBER_FIELD(m_yawVelocity),
	mxMEMBER_FIELD(m_pitchAngle),
	mxMEMBER_FIELD(m_yawAngle),

	mxMEMBER_FIELD(m_pitchSensitivity),
	mxMEMBER_FIELD(m_yawSensitivity),

	mxMEMBER_FIELD(m_minPitch),
	mxMEMBER_FIELD(m_maxPitch),
mxEND_REFLECTION
FlyingCamera::FlyingCamera()
{
	m_eyePosition		= Float3_Zero();
	m_upDirection		= Float3_Set(0,0,1);
	m_lookDirection		= Float3_Set(0,1,0);
	m_rightDirection	= Float3_Set(1,0,0);

	m_linearVelocity	= Float3_Zero();
	m_linearFriction	= 0.25f;
	m_angularFriction	= 0.25f;

	m_movementSpeed		= 1.0f;
	m_strafingSpeed		= 1.0f;
	m_rotationSpeed		= 0.003f;

	m_hasLinearMomentum	= false;
	m_hasAngularMomentum= false;
	m_invertPitch		= false;
	m_invertYaw			= false;

	m_pitchVelocity	= 0.0f;
	m_yawVelocity	= 0.0f;
	m_pitchAngle	= 0.0f;
	m_yawAngle		= 0.0f;

	m_pitchSensitivity = 1.0f;
	m_yawSensitivity = 1.0f;

	m_minPitch = mxHALF_PI * -0.9f;
	m_maxPitch = mxHALF_PI * 0.9f;

	g_movementFlags = 0;
}
void FlyingCamera::Set( const Float3& position, const Float3& lookDirection )
{
	mxASSERT(Float3_Length(lookDirection) > 1e-5f);
	// Remember, camera points down +Y of local axes!
	Float3	upDirection = Float3_Set(0,0,1);

	Float3	axisX;	// camera right vector
	Float3	axisY;	// camera forward vector
	Float3	axisZ;	// camera up vector

	axisY = Float3_Normalized( lookDirection );

	axisX = Float3_Cross( axisY, upDirection );
	axisX = Float3_Normalized( axisX );

	axisZ = Float3_Cross( axisX, axisY );
	axisZ = Float3_Normalized( axisZ );

	m_eyePosition = position;
	m_upDirection = axisZ;
	m_lookDirection = axisY;
	m_rightDirection = axisX;

	// figure out the yaw/pitch of the camera

	// For any given angular displacement, there are an infinite number of Euler angle representations
	// due to Euler angle aliasing. The technique we are using here will always return 'canonical' Euler angles,
	// with heading and bank in range +/- 180° and pitch in range +/- 90°.

    // To figure out the yaw/pitch of the camera, we just need the Y basis vector.
	// An axis vector can be rotated into the direction
	// by first rotating along the world X axis by the pitch, then by the Z by the yaw.
	// The two-argument form of Arc Tangent is very useful to convert rectangular to polar coordinates.
	// Here we assume that the forward (Y axis) vector is normalized.
	m_pitchAngle = Float_ASin( axisY.z );	// [–Pi/2 .. +Pi/2] radians
	m_yawAngle = -Float_ATan2( axisY.x, axisY.y );	// [–Pi .. +Pi] radians
}
void FlyingCamera::Update( float deltaSeconds )
{
	// Update the camera rotation

	// Update the pitch and yaw angle based on mouse movement
	float pitchDelta = m_pitchVelocity * (deltaSeconds * m_rotationSpeed);
	float yawDelta = m_yawVelocity * (deltaSeconds * m_rotationSpeed);	

	if( m_hasAngularMomentum ) {
		m_pitchVelocity *= (1.0f - m_angularFriction) * deltaSeconds;
		m_yawVelocity *= (1.0f - m_angularFriction) * deltaSeconds;
	} else {
		m_pitchVelocity = 0.0f;
		m_yawVelocity = 0.0f;
	}

	// Invert pitch if requested
	if( m_invertPitch ) {
		pitchDelta = -pitchDelta;
	}
	if( m_invertYaw ) {
		yawDelta = -yawDelta;
	}
	m_pitchAngle += pitchDelta;
	m_yawAngle += yawDelta;

	// Limit pitch to straight up or straight down
	m_pitchAngle = clampf( m_pitchAngle, m_minPitch, m_maxPitch );

	// Make a rotation matrix based on the camera's yaw and pitch.

	// An axis vector can be rotated into the direction
	// by first rotating along the world X axis by the pitch, then by the Z by the yaw.
	const Float3x3	R = Float3x3_Multiply(
		Float3x3_RotationX(m_pitchAngle), Float3x3_RotationZ(m_yawAngle)
	);
	mxTODO(debug matrix);
	//const Float3x3	R = Float3x3_RotationPitchRollYaw( m_pitchAngle, 0.0f, m_yawAngle );
	mxTODO(debug quaternion);
	//const Vector4	Q = Quaternion_RotationPitchRollYaw( m_pitchAngle, 0.0f, m_yawAngle );
	//const Float3x3	R = Float3x3_FromQuaternion(Q);

	m_rightDirection	= Float3x3_Transform( R, Float4_As_Float3(g_Float4_UnitX) );
	m_lookDirection		= Float3x3_Transform( R, Float4_As_Float3(g_Float4_UnitY) );
	m_upDirection		= Float3x3_Transform( R, Float4_As_Float3(g_Float4_UnitZ) );	


	// Update the camera position
	if( Float3_Length(m_linearVelocity) > 1.0f ) {
		m_linearVelocity = Float3_Normalized(m_linearVelocity);
	}

	m_eyePosition += m_linearVelocity * (m_movementSpeed * deltaSeconds);

	if( m_hasLinearMomentum ) {
		m_linearVelocity *= (1.0f - m_linearFriction) * deltaSeconds;
	} else {
		m_linearVelocity = Float3_Zero();
	}

	mxASSERT(Float3_IsNormalized(m_rightDirection));
	mxASSERT(Float3_IsNormalized(m_lookDirection));
	mxASSERT(Float3_IsNormalized(m_upDirection));


	//Float3 keyboardDirection;// Direction vector of keyboard input
	//keyboardDirection = Float3_Zero();
	if( g_movementFlags & FlyingCamera::FORWARD ) {
		m_linearVelocity += m_lookDirection;
	}
	if( g_movementFlags & FlyingCamera::BACKWARD ) {
		m_linearVelocity -= m_lookDirection;
	}
	if( g_movementFlags & FlyingCamera::LEFT ) {
		m_linearVelocity -= m_rightDirection;
	}
	if( g_movementFlags & FlyingCamera::RIGHT ) {
		m_linearVelocity += m_rightDirection;
	}
	if( g_movementFlags & FlyingCamera::UP ) {
		m_linearVelocity += m_upDirection;
	}
	if( g_movementFlags & FlyingCamera::DOWN ) {
		m_linearVelocity -= m_upDirection;
	}
	//// get local-space normalized 'acceleration' from input controls
	//Float3 acceleration = keyboardDirection;
	//if( Float3_Length(acceleration) > 1.0f ) {
	//	acceleration = Float3_Normalized(acceleration);
	//}
}
//void FlyingCamera::Forward( float units )
//{
//	m_linearVelocity += m_lookDirection * units;
//}
//void FlyingCamera::Strafe( float units )
//{
//	m_linearVelocity += m_rightDirection * units;
//}
void FlyingCamera::Pitch( float units )
{
	m_pitchVelocity += units * m_pitchSensitivity;
}
//void FlyingCamera::Down( float units )
//{
//	m_linearVelocity -= m_upDirection * units;
//}
void FlyingCamera::Yaw( float units )
{
	m_yawVelocity += units * m_yawSensitivity;
}
//void FlyingCamera::Up( float units )
//{
//	m_linearVelocity += m_upDirection * units;
//}
Float4x4 FlyingCamera::BuildViewMatrix() const
{
	mxASSERT(Float3_IsNormalized(m_rightDirection));
	mxASSERT(Float3_IsNormalized(m_lookDirection));
	mxASSERT(Float3_IsNormalized(m_upDirection));
	Float4x4	cameraWorldMatrix;
	cameraWorldMatrix.r0 = LoadFloat3( m_rightDirection, 0.0f );	// X
	cameraWorldMatrix.r1 = LoadFloat3( m_lookDirection,  0.0f );	// Y
	cameraWorldMatrix.r2 = LoadFloat3( m_upDirection,    0.0f );	// Z
	cameraWorldMatrix.r3 = LoadFloat3( m_eyePosition,    1.0f );	// T
	return Matrix_OrthoInverse( cameraWorldMatrix );
}

void FlyingCamera::move_forward( GameActionID action, EInputState status, float value )
{
	setbit_cond(g_movementFlags, ( status == IS_Pressed || status == IS_HeldDown ), FlyingCamera::FORWARD);
}
void FlyingCamera::move_back( GameActionID action, EInputState status, float value )
{
	setbit_cond(g_movementFlags, ( status == IS_Pressed || status == IS_HeldDown ), FlyingCamera::BACKWARD);
}
void FlyingCamera::strafe_left( GameActionID action, EInputState status, float value )
{
	setbit_cond(g_movementFlags, ( status == IS_Pressed || status == IS_HeldDown ), FlyingCamera::LEFT);
}
void FlyingCamera::strafe_right( GameActionID action, EInputState status, float value )
{
	setbit_cond(g_movementFlags, ( status == IS_Pressed || status == IS_HeldDown ), FlyingCamera::RIGHT);
}
void FlyingCamera::move_up( GameActionID action, EInputState status, float value )
{
	setbit_cond(g_movementFlags, ( status == IS_Pressed || status == IS_HeldDown ), FlyingCamera::UP);
}
void FlyingCamera::move_down( GameActionID action, EInputState status, float value )
{
	setbit_cond(g_movementFlags, ( status == IS_Pressed || status == IS_HeldDown ), FlyingCamera::DOWN);
}
void FlyingCamera::rotate_pitch( GameActionID action, EInputState status, float value )
{
	//DBGOUT("rotate_pitch\n");
	Pitch(value);
}
void FlyingCamera::rotate_yaw( GameActionID action, EInputState status, float value )
{
	//DBGOUT("rotate_yaw\n");
	Yaw(value);
}
