#include "types.h"

void GScr_getTypeIndex() {
    if( Scr_GetNumParam() != 1 ) {
        Scr_Error( "Usage: getTypeIndex( <var> )" );
        return;
    }

    Scr_AddInt(Scr_GetType( 0 ));
}

void GScr_getType() {
    if( Scr_GetNumParam() != 1 ) {
        Scr_Error( "Usage: getType( <var> )" );
        return;
    }

    int value;
    switch( Scr_GetType( 0 ) ) {
        case 1:
            Scr_AddString("object");
            break;
        case 2: 
            Scr_AddString("string");
            break;
        case 3:
            Scr_AddString("localized_string");
            break;
        case 5:
            Scr_AddString("float");
            break;
        case 6: 
            value = Scr_GetInt( 0 );
            if( value == 0 || value == 1 ) {
                Scr_AddString("bool");
            } else {
                Scr_AddString("int");
            }
            break;
        case 9:
            Scr_AddString("function");
            break;
        default:
            Scr_AddString("unknown");
            break;
    }
}

void GScr_isFunction() {
    if( Scr_GetNumParam() != 1 ) {
        Scr_Error( "Usage: isFunction( <var> )" );
        return;
    }

    Scr_AddBool(Scr_GetType( 0 ) == 9);
}

void GScr_isLocalizedString() {
    if( Scr_GetNumParam() != 1 ) {
        Scr_Error( "Usage: isLocalizedString( <var> )" );
        return;
    }

    Scr_AddBool(Scr_GetType( 0 ) == 3);
}

void GScr_isInt() {
    if( Scr_GetNumParam() != 1 ) {
        Scr_Error( "Usage: isInt( <var> )" );
        return;
    }

    Scr_AddBool(Scr_GetType( 0 ) == 6);
}

void GScr_isFloat() {
    if( Scr_GetNumParam() != 1 ) {
        Scr_Error( "Usage: isFloat( <var> )" );
        return;
    }

    Scr_AddBool(Scr_GetType( 0 ) == 5);
}

void GScr_isBool() {
    if( Scr_GetNumParam() != 1 ) {
        Scr_Error( "Usage: isBool( <var> )" );
        return;
    }

    if( Scr_GetType( 0 ) != 6 ) {
        Scr_AddBool( qfalse );
        return;
    }

    int value = Scr_GetInt( 0 );
    Scr_AddBool(value == 0 || value == 1);
}

// Objects are fucking weird
void GScr_isObject() {
    if( Scr_GetNumParam() != 1 ) {
        Scr_Error( "Usage: isObject( <var> )" );
        return;
    }

    Scr_AddBool(Scr_GetType( 0 ) == 1);
}