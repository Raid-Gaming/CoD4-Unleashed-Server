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
        case 4: 
            Scr_AddString("vector");
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

void isCheck(int condition, char* funcName) {
    if( Scr_GetNumParam() != 1 ) {
        char* error = "";
        sprintf(error, "Usage: %s( <var> )", funcName);
        Scr_Error( error );
        return;
    }

    Scr_AddBool(condition);
}

void GScr_isFunction() {
    isCheck(Scr_GetType( 0 ) == 9, "isFunction");
}

void GScr_isVector() {
    isCheck(Scr_GetType( 0 ) == 4, "isVector");
}

void GScr_isLocalizedString() {
    isCheck(Scr_GetType( 0 ) == 3, "isLocalizedString");
}

void GScr_isInt() {
    isCheck(Scr_GetType( 0 ) == 6, "isInt");
}

void GScr_isFloat() {
    isCheck(Scr_GetType( 0 ) == 5, "isFloat");
}

// Objects are fucking weird
void GScr_isObject() {
    isCheck(Scr_GetType( 0 ) == 1, "isObject");
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