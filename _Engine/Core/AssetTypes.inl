//DECLARE_ASSET_TYPE( id, class, file_extension, version, description )
#ifdef DECLARE_ASSET_TYPE

DECLARE_ASSET_TYPE( UNKNOWN,	0,									"unknown",	0, "" )

DECLARE_ASSET_TYPE( FONT,		0,									"font",		0, "" )

DECLARE_ASSET_TYPE( MESH,		mxEXTRACT_TYPE_GUID(rxMesh),		"mesh",		0, "" )

DECLARE_ASSET_TYPE( SHADER,		mxEXTRACT_TYPE_GUID(FxShader),		"shader",	0, "" )

DECLARE_ASSET_TYPE( TEXTURE,	mxEXTRACT_TYPE_GUID(rxTexture),		"texture",	0, "" )

// a graphics material
DECLARE_ASSET_TYPE( MATERIAL,	mxEXTRACT_TYPE_GUID(rxMaterial),	"material",	0, "" )

DECLARE_ASSET_TYPE( SCRIPT,		mxEXTRACT_TYPE_GUID(Script),		"script",	0, "" )

//DECLARE_ASSET_TYPE( CLUMP,		mxEXTRACT_TYPE_GUID(Clump),			"clump",	0, "" )

// a simple config for data-driven rendering (similar to Horde3D's pipeline)
DECLARE_ASSET_TYPE( PIPELINE,	mxEXTRACT_TYPE_GUID(FxLibrary),		"renderer",	0, "" )

#endif // DECLARE_ASSET_TYPE
