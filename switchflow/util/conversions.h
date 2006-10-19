#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#define ISDIGIT(x)   ( (x) >= '0' && (x) <= '9' )

#define ISHEXCHAR(x) ( ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F') )
#define ISHEX(x)     ( ISDIGIT(x) || ISHEXCHAR(x) )

#define HC2INT(x)    ( ((x) >= 'a' && (x) <= 'f') ? (x) - 'a' + 10 :                    \
                        ( ((x) >= 'A' && (x) <= 'F') ? (x) - 'A' + 10 : (x - '0') ) )


#endif // CONVERSIONS_H
