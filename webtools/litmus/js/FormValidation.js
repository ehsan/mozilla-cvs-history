var iBugNumber = "This field must be a valid, positive integer (>0). Please re-enter it now.";
var iEmail = "This field must be a valid email address (like foo@bar.com). Please re-enter it now.";
var iBuildId = "This field must be a valid build id, which is a string of 8 digits most easily found in the About dialog. Please re-enter it now.";
var defaultEmptyOK = false;
var mPrefix = "You did not enter a value into the ";
var mSuffix = " field. This is a required field. Please enter it now.";
var whitespace = " \t\n\r";

// Check whether string s is empty.
function isEmpty(s)
{
    return ((s == null) || (s.length == 0));
}

// Returns true if string s is empty or 
// whitespace characters only.
function isWhitespace (s)
{
    var i;
    
    // Is s empty?
    if (isEmpty(s)) return true;
    
    // Search through string's characters one by one
    // until we find a non-whitespace character.
    // When we do, return false; if we don't, return true.
    
    for (i = 0; i < s.length; i++) {   
        // Check that current character isn't whitespace.
	var c = s.charAt(i);
	
	if (whitespace.indexOf(c) == -1) return false;
    }
    
    // All characters are whitespace.
    return true;
}

// Returns true if character c is a digit 
// (0 .. 9).
function isDigit (c)
{   
    return ((c >= "0") && (c <= "9"));
}

// Notify user that required field theField is empty.
// String s describes expected contents of theField.value.
// Put focus in theField and return false.
function warnEmpty (theField, s)
{
    theField.focus();
    alert(mPrefix + s + mSuffix); 
    return false;
}

// Notify user that contents of field theField are invalid.
// String s describes expected contents of theField.value.
// Put select theField, put focus in it, and return false.
function warnInvalid (theField, s)
{
    theField.focus();
    theField.select();
    alert(s);
    return false;
}

// isEmail (STRING s [, BOOLEAN emptyOK])
// 
// Email address must be of form a@b.c -- in other words:
// * there must be at least one character before the @
// * there must be at least one character before and after the .
// * the characters @ and . are both required
//
// For explanation of optional argument emptyOK,
// see comments of function isInteger.
function isEmail (s)
{
    if (isEmpty(s)) {
	if (isEmail.arguments.length == 1) {
	    return defaultEmptyOK;
	} else { 
	    return (isEmail.arguments[1] == true);
	}
    }
    
    // is s whitespace?
    if (isWhitespace(s)) { 
	return false;
    }
    
    // there must be >= 1 character before @, so we
    // start looking at character position 1 
    // (i.e. second character)
    var i = 1;
    var sLength = s.length;
    
    // look for @
    while ((i < sLength) && (s.charAt(i) != "@")) { 
	i++;
    }
    
    if ((i >= sLength) || (s.charAt(i) != "@")) {
	return false;
    } else {
	i += 2;
    }
    
    // look for .
    while ((i < sLength) && (s.charAt(i) != ".")) {
	i++;
    }
    
    // there must be at least one character after the .
    if ((i >= sLength - 1) || (s.charAt(i) != ".")) {
	return false;
    } else {    
	return true;
    }
}

// checkString (TEXTFIELD theField, STRING s, [, BOOLEAN emptyOK==false])
//
// Check that string theField.value is not all whitespace.
//
// For explanation of optional argument emptyOK,
// see comments of function isInteger.
function checkString (theField, s, emptyOK)
{   
    // Next line is needed on NN3 to avoid "undefined is not a number" error
    // in equality comparison below.
    if (checkString.arguments.length == 2) {
	emptyOK = defaultEmptyOK;
    }
    if ((emptyOK == true) && (isEmpty(theField.value))) {
	return true;
    }
    if (isWhitespace(theField.value)) {
	return warnEmpty(theField, s);
    } else {    
	return true;
    }
}

// checkEmail (TEXTFIELD theField [, BOOLEAN emptyOK==false])
//
// Check that string theField.value is a valid Email.
//
// For explanation of optional argument emptyOK,
// see comments of function isInteger.
function checkEmail (theField, emptyOK)
{   
    if (checkEmail.arguments.length == 1) { 
	emptyOK = defaultEmptyOK;
    }
    
    if ((emptyOK == true) && (isEmpty(theField.value))) {
	return true;
    } else {
	if (!isEmail(theField.value, false)) {
	    return warnInvalid (theField, iEmail);
	} else {
	    return true;
	}
    }
}

// checkBuildId (TEXTFIELD theField [, BOOLEAN emptyOK==false])
//
// Check that string theField.value is a valid build id.
//
// For explanation of optional argument emptyOK,
// see comments of function isInteger.
function checkBuildId (theField, emptyOK)
{   
    if (checkBuildId.arguments.length == 1) { 
	emptyOK = defaultEmptyOK;
    }
    
    if ((emptyOK == true) && (isEmpty(theField.value))) {
	return true;
    } else {
	if (!/\d{8,}/.test(theField.value)) {
	    return warnInvalid (theField, iBuildId);
	} else {
	    return true;
	}
    }
}


// isInteger (STRING s [, BOOLEAN emptyOK])
// 
// Returns true if all characters in string s are numbers.
//
// Accepts non-signed integers only. Does not accept floating 
// point, exponential notation, etc.
//
// We don't use parseInt because that would accept a string
// with trailing non-numeric characters.
//
// By default, returns defaultEmptyOK if s is empty.
// There is an optional second argument called emptyOK.
// emptyOK is used to override for a single function call
//      the default behavior which is specified globally by
//      defaultEmptyOK.
// If emptyOK is false (or any value other than true), 
//      the function will return false if s is empty.
// If emptyOK is true, the function will return true if s is empty.
//
// EXAMPLE FUNCTION CALL:     RESULT:
// isInteger ("5")            true 
// isInteger ("")             defaultEmptyOK
// isInteger ("-5")           false
// isInteger ("", true)       true
// isInteger ("", false)      false
// isInteger ("5", false)     true
function isInteger (s)
{
    var i;
    
    if (isEmpty(s)) 
       if (isInteger.arguments.length == 1) return defaultEmptyOK;
       else return (isInteger.arguments[1] == true);

    // Search through string's characters one by one
    // until we find a non-numeric character.
    // When we do, return false; if we don't, return true.

    for (i = 0; i < s.length; i++)
    {   
        // Check that current character is number.
        var c = s.charAt(i);

        if (!isDigit(c)) return false;
    }

    // All characters are numbers.
    return true;
}

// isSignedInteger (STRING s [, BOOLEAN emptyOK])
// 
// Returns true if all characters are numbers; 
// first character is allowed to be + or - as well.
//
// Does not accept floating point, exponential notation, etc.
//
// We don't use parseInt because that would accept a string
// with trailing non-numeric characters.
//
// For explanation of optional argument emptyOK,
// see comments of function isInteger.
//
// EXAMPLE FUNCTION CALL:          RESULT:
// isSignedInteger ("5")           true 
// isSignedInteger ("")            defaultEmptyOK
// isSignedInteger ("-5")          true
// isSignedInteger ("+5")          true
// isSignedInteger ("", false)     false
// isSignedInteger ("", true)      true
function isSignedInteger (s)
{
    if (isEmpty(s)) 
	if (isSignedInteger.arguments.length == 1) return defaultEmptyOK;
	else return (isSignedInteger.arguments[1] == true);
    
    else {
	var startPos = 0;
	var secondArg = defaultEmptyOK;
	
	if (isSignedInteger.arguments.length > 1)
	    secondArg = isSignedInteger.arguments[1];
	
	// skip leading + or -
	if ( (s.charAt(0) == "-") || (s.charAt(0) == "+") )
	    startPos = 1;    
	return (isInteger(s.substring(startPos, s.length), secondArg));
    }
}


// isPositiveInteger (STRING s [, BOOLEAN emptyOK])
// 
// Returns true if string s is an integer > 0.
//
// For explanation of optional argument emptyOK,
// see comments of function isInteger.
function isPositiveInteger (s)
{
    var secondArg = defaultEmptyOK;

    if (isPositiveInteger.arguments.length > 1)
        secondArg = isPositiveInteger.arguments[1];

    // The next line is a bit byzantine.  What it means is:
    // a) s must be a signed integer, AND
    // b) one of the following must be true:
    //    i)  s is empty and we are supposed to return true for
    //        empty strings
    //    ii) this is a positive, not negative, number

    return (isSignedInteger(s, secondArg)
         && ( (isEmpty(s) && secondArg)  || (parseInt (s) > 0) ) );
}


