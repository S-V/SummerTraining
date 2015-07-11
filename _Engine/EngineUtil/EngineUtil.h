#pragma once

#if MX_AUTOLINK
#pragma comment( lib, "EngineUtil.lib" )
#endif //MX_AUTOLINK

#include <Core/Client.h>
#include <Core/VectorMath.h>

//--------------------------------------------------------------------------------------
//	ModelViewerCamera
//	Simple model viewing camera class that rotates around the object.
//--------------------------------------------------------------------------------------
struct OrbitingCamera: public CStruct {
	ArcBall		arcBall;
	float		radius;
public:
	mxDECLARE_CLASS(OrbitingCamera,CStruct);
	mxDECLARE_REFLECTION;
	OrbitingCamera();

	//void Initialize( int width, int height );	// must be done everytime after viewport resizing
	//void BeginDrag( int x, int y );	// start the rotation (pass current mouse position)
	//void MoveDrag( int x, int y );	// continue the rotation (pass current mouse position)
	//void EndDrag();					// stop the rotation 

	//void SetOrigin( const glm::vec3& center );
	//void SetRadius( float radius );

	Float4x4 GetTransform() const;

	void OnInputEvent( const SInputEvent& _event );
};


// code and comments stolen from:
// https://github.com/slicedpan/NarrowPhase/blob/master/FPSCamera/FPSCamera.h
struct FlyingCamera : CStruct
{
	Float3	m_eyePosition;		// camera position
	Float3	m_upDirection;		// Z-axis of camera's world matrix (up direction)
	Float3	m_lookDirection;	// Y-axis of camera's world matrix (forward direction)
	Float3	m_rightDirection;	// X-axis of camera's world matrix (right direction)

	Float3	m_linearVelocity;	// movement velocity accumulator
	float	m_linearFriction;
	float	m_angularFriction;

	float	m_movementSpeed;	// forward walking speed [units/s]
	float	m_strafingSpeed;	// translation speed [units/s]
	float	m_rotationSpeed;	// rotation/look speed [radians/s]

	bool	m_hasLinearMomentum;
	bool	m_hasAngularMomentum;
	bool	m_invertPitch;	// invert vertical mouse movement?
	bool	m_invertYaw;	// invert horizontal mouse movement?

	float	m_pitchVelocity;	// pitch velocity accumulator
	float	m_yawVelocity;		// yaw velocity accumulator
	float	m_pitchAngle;		// current pitch angle in radians
	float	m_yawAngle;			// current yaw angle in radians (heading)

	float	m_pitchSensitivity;
	float	m_yawSensitivity;

	// for clamping speed and angles
	//float	m_minSpeed;
	float	m_minPitch;	// usually Half_PI * (-1)
	float	m_maxPitch;	// usually Half_PI

	UINT32 g_movementFlags;	// EMovementFlags

public:
	mxDECLARE_CLASS(FlyingCamera,CStruct);
	mxDECLARE_REFLECTION;
	FlyingCamera();

	void Set( const Float3& position, const Float3& lookDirection );

	// don't forget to periodically call this function
	void Update( float elapsedSeconds );

	//void Forward( float units );
	//void Strafe( float units );
	//void Pitch( float units );
	//void Down( float units );
	//void Yaw( float units );
	//void Up( float units );

	void Pitch( float units );
	void Yaw( float units );

	Float4x4 BuildViewMatrix() const;

	enum EMovementFlags {
		FORWARD	= BIT(0),
		BACKWARD= BIT(1),
		LEFT	= BIT(2),
		RIGHT	= BIT(3),
		UP		= BIT(4),
		DOWN	= BIT(5),
		RESET	= BIT(6),	// reset position and orientation
	};

	// Input bindings
	void move_forward( GameActionID action, EInputState status, float value );
	void move_back( GameActionID action, EInputState status, float value );
	void strafe_left( GameActionID action, EInputState status, float value );
	void strafe_right( GameActionID action, EInputState status, float value );
	void move_up( GameActionID action, EInputState status, float value );
	void move_down( GameActionID action, EInputState status, float value );
	void rotate_pitch( GameActionID action, EInputState status, float value );
	void rotate_yaw( GameActionID action, EInputState status, float value );
};
