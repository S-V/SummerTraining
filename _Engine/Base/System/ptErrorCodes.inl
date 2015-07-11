//mxDECLARE_ERROR_CODE( ENUM, NUMBER, STRING )

// Success codes:
// the function returned no error, all is well.
mxDECLARE_ERROR_CODE( ALL_OK,		0, "No error, all is well" )

// Failure codes:

// NOTE: avoid using this code, always try to give meaningful error info.
mxDECLARE_ERROR_CODE( ERR_UNKNOWN_ERROR, 			1,		"Unknown error" )

mxDECLARE_ERROR_CODE( ERR_NOT_IMPLEMENTED,			2,		"Not implemented" )
mxDECLARE_ERROR_CODE( ERR_UNSUPPORTED_FEATURE,		3,		"Unsupported feature" )
mxDECLARE_ERROR_CODE( ERR_UNSUPPORTED_VERSION,		5,		"Unsupported version" )// File version not supported (anymore or yet)
mxDECLARE_ERROR_CODE( ERR_INCOMPATIBLE_VERSION,		4,		"Incompatible version" )
mxDECLARE_ERROR_CODE( ERR_FEATURE_NOT_AVAILABLE,	6,		"Feature not available" )// The function could not complete because it tries to use a feature (or set of features) not currently supported.

// general errors
mxDECLARE_ERROR_CODE( ERR_INVALID_PARAMETER, 		50,		"Invalid argument" )	// An invalid parameter was passed to the returning function.
mxDECLARE_ERROR_CODE( ERR_INDEX_OUT_OF_RANGLE, 		51,		"Index out of range" )	// Index value outside the valid range
mxDECLARE_ERROR_CODE( ERR_INVALID_FUNCTION_CALL, 	52,		"Invalid function call" )
mxDECLARE_ERROR_CODE( ERR_NULL_POINTER_PASSED, 		53,		"Null pointer argument" )
mxDECLARE_ERROR_CODE( ERR_FAILED_TO_LOAD_LIBRARY, 	54,		"Failed to load dynamic library" )

// Object-related
mxDECLARE_ERROR_CODE( ERR_OBJECT_NOT_FOUND, 		100,		"Object not found" )
mxDECLARE_ERROR_CODE( ERR_OBJECT_OF_WRONG_TYPE, 	101,		"Object of wrong type" )
mxDECLARE_ERROR_CODE( ERR_NAME_ALREADY_TAKEN, 		102,		"Object with the given name already exists" )
mxDECLARE_ERROR_CODE( ERR_TOO_MANY_OBJECTS, 		103,		"There are too many unique instances of a particular type of object" )
mxDECLARE_ERROR_CODE( ERR_SUCH_OBJECT_ALREADY_EXISTS, 	104,	"Such object already exists" )

// memory errors
mxDECLARE_ERROR_CODE( ERR_OUT_OF_MEMORY, 			150,		"Out of memory" )//Operation failed due to insufficient memory.
mxDECLARE_ERROR_CODE( ERR_STACK_OVERFLOW, 			151,		"Stack overflow" )
mxDECLARE_ERROR_CODE( ERR_STACK_UNDERFLOW, 			152,		"Stack underflow" )
mxDECLARE_ERROR_CODE( ERR_BUFFER_TOO_SMALL, 		153,		"Buffer too small" )
mxDECLARE_ERROR_CODE( ERR_INVALID_ALIGNMENT, 		154,		"Invalid alignment" )

// I/O errors
mxDECLARE_ERROR_CODE( ERR_FAILED_TO_PARSE_DATA, 	200,		"Failed to parse data" )
mxDECLARE_ERROR_CODE( ERR_FAILED_TO_CREATE_FILE, 	201,		"Failed to create file" )
mxDECLARE_ERROR_CODE( ERR_FAILED_TO_OPEN_FILE, 		202,		"Failed to open file" )
mxDECLARE_ERROR_CODE( ERR_FAILED_TO_READ_FILE,		203,		"Failed to read file" )
mxDECLARE_ERROR_CODE( ERR_FAILED_TO_WRITE_FILE, 	204,		"Failed to write file" )
mxDECLARE_ERROR_CODE( ERR_FAILED_TO_SEEK_FILE, 		205,		"Failed to seek file" )
mxDECLARE_ERROR_CODE( ERR_FILE_HAS_ZERO_SIZE, 		206,		"File has zero size" )
mxDECLARE_ERROR_CODE( ERR_FILE_OR_PATH_NOT_FOUND, 	207,		"File or path not found" )

// Miscellaneous errors
//-- Scripts
mxDECLARE_ERROR_CODE( ERR_SYNTAX_ERROR, 			500,		"Syntax error" )
mxDECLARE_ERROR_CODE( ERR_COMPILATION_FAILED, 		501,		"Compilation failed" )
mxDECLARE_ERROR_CODE( ERR_LINKING_FAILED, 			502,		"Linking failed" )
mxDECLARE_ERROR_CODE( ERR_VALIDATION_FAILED, 		503,		"Validation failed" )
mxDECLARE_ERROR_CODE( ERR_RUNTIME_ERROR, 			504,		"Run-time error" )
mxDECLARE_ERROR_CODE( ERR_WHILE_ERROR_HANDLER, 		505,		"Error while running the error handler" )
//-- Authentication
mxDECLARE_ERROR_CODE( ERR_WRONG_PASSWORD, 			506,		"Wrong password" )

//mxDECLARE_ERROR_CODE( ERR_FAILED_TO_LOAD_ASSET, 	XXX,		"Asset not found" )
