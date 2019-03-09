/*

=== CoD4: Unleashed ===

Module: JSON
Author: atrX

This module allows conversion from JSON strings to an array and vice-versa

*/

/*
Encode an array to a JSON string

Usage:
arr = [];
arr[arr.size] = "abc";
arr[arr.size] = "123";
arr[arr.size] = "xyz";
str = unleashed\json::encode(arr);
*/
encode(obj) {
	str = "";
	keys = getArrayKeys(obj);

	isObject = false;
	for (i = 0; i < keys.size; i++) {
		if (isString(keys[i]) || keys[i] < 0 || keys[i] >= keys.size) {
			isObject = true;
			break;
		}
	}

	if (isObject) {
		str += "{";
		for (i = 0; i < keys.size; i++) {
			value = obj[keys[i]];
			str += "\"" + keys[i] + "\":";
			if (isArray(value)) {
				str += encode(value);
			} else if (isString(value)) {
				str += "\"" + value + "\"";
			} else {
				str += value;
			}
			
			if (i < keys.size - 1) {
				str += ",";
			}
		}
		str += "}";
	} else {
		str += "[";
		for (i = 0; i < keys.size; i++) {
			if (isArray(obj[i])) {
				str += encode(obj[i]);
			} else if (isString(obj[i])) {
				str += "\"" + obj[i] + "\"";
			} else {
				str += obj[i];
			}
			
			if (i < keys.size - 1) {
				str += ",";
			}
		}
		str += "]";
	}

	return str;
}

/*
Decode a JSON string to an array

Usage: obj = unleashed\json::decode("{\"msg\": \"Hello world!\"}");
*/
decode(str) {
	obj = [];
	
	// Are we dealing with an object or array?
	if (str[0] == "{") {
		// Loop through the entire string
		for (i = 1; i < str.size - 1; i++) {
			if (str[i] == ",") {
				i++;
			}

			// Find key
			key = "";
			for (i++; str[i] != "\""; i++) {
				key += str[i];
			}

			// Find value
			value = "";
			i += 2;

			switch (str[i]) {
			case "{":
				inString = false;
				bracketCount = 0;
				value += "{";
				for (i++; str[i] != "}" || bracketCount > 0 || inString; i++) {
					if (str[i] == "\\") {
						value += str[i];
						i++;
					} else if (!inString && str[i] == "{") {
						bracketCount++;
					} else if (!inString && str[i] == "}") {
						bracketCount--;
					} else if (str[i] == "\"") {
						inString = !inString;
					}
					value += str[i];
				}
				value += "}";
				obj[key] = decode(value);
				break;
			
			case "[":
				inString = false;
				bracketCount = 0;
				value += "[";
				for (i++; str[i] != "]" || bracketCount > 0 || inString; i++) {
					if (str[i] == "\\") {
						value += str[i];
						i++;
					} else if (!inString && str[i] == "[") {
						bracketCount++;
					} else if (!inString && str[i] == "]") {
						bracketCount--;
					} else if (str[i] == "\"") {
						inString = !inString;
					}
					value += str[i];
				}
				value += "]";
				obj[key] = decode(value);
				break;
			
			case "\"":
				for (i++; str[i] != "\""; i++) {
					if (str[i] == "\\") {
						value += str[i];
						i++;
					}
					value += str[i];
				}
				obj[key] = value;
				break;
			
			default:
				for (; str[i] != "," && str[i] != "}" && str[i] != "]"; i++) {
					value += str[i];
				}
				obj[key] = toFloat(value);
				break;
			}
		}
	} else {
		// Loop through the entire string
		for (i = 1; i < str.size - 1; i++) {
			if (str[i] == ",") {
				i++;
			}

			// Find value
			value = "";
			inString = false;
			objCount = 0;
			arrCount = 0;
			for (; (str[i] != "," && str[i] != "]") || inString || objCount > 0 || arrCount > 0; i++) {
				if (str[i] == "\"") {
					inString = !inString;
				} else if (!inString && str[i] == "{") {
					objCount++;
				} else if (!inString && str[i] == "}") {
					objCount--;
				} else if (!inString && str[i] == "[") {
					arrCount++;
				} else if (!inString && str[i] == "]") {
					arrCount--;
				}
				value += str[i];
			}
			
			switch (value[0]) {
			case "{": case "[":
				obj[obj.size] = decode(value);
				break;
			
			case "\"":
				obj[obj.size] = getSubStr(value, 1, value.size - 1);
				break;
			
			default:
				obj[obj.size] = toFloat(value);
				break;
			}
		}
	}

	return obj;
}
