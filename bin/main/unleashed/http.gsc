/*

=== CoD4: Unleashed ===

Module: HTTP
Author: atrX

This module acts as a facade around the HTTP functionality of the CoD4: Unleashed patch.
It is fully optional and merely serves as a more developer-friendly interface.

*/

/*
Set an option to use for all HTTP requests.

Currently supported options:
- http_base_path
- http_headers

Usage:
unleashed\http::setOpt("http_base_path", "http://localhost");
*/
setOpt(key, value) {
	key = toLower(key);
	if (!isDefined(level._unleashed)) {
		level._unleashed = [];
	}
	level._unleashed[key] = value;
}

/*
Get the value of a global HTTP request option.

Usage:
opt = unleashed\http::getOpt("http_headers");
*/
getOpt(key) {
	key = toLower(key);
	if (!isDefined(level._unleashed)) {
		level._unleashed = [];
	}
	return level._unleashed[key];
}

/*
If called on an HTTP request object:
Sets a header on an HTTP request object.

Usage:
request unleashed\http::setHeader("Content-type", "application/json");


Otherwise:
Sets a header for use with all HTTP requests.

Usage:
unleashed\http::setHeader("Content-type", "application/json");
*/
setHeader(key, value) {
	if (isDefined(self) && isDefined(self._unleashedHttp)) {
		self _setObjectHeader(key, value);
	} else {
		_setGlobalHeader(key, value);
	}
}

_setObjectHeader(key, value) {
	if (!isDefined(self.headers)) {
		self.headers = [];
	}
	self.headers[key] = value;
}

_setGlobalHeader(key, value) {
	key = toLower(key);
	if (!isDefined(level._unleashed)) {
		level._unleashed = [];
	}
	if (!isDefined(level._unleashed["http_headers"])) {
		level._unleashed["http_headers"] = [];
	}

	index = level._unleashed["http_headers"].size;
	for (i = 0; i < level._unleashed["http_headers"].size; i++) {
		if (strTok(level._unleashed["http_headers"][i], ": ")[0] == key) {
			index = i;
			break;
		}
	}
	level._unleashed["http_headers"][index] = "" + key + ": " + value;
}

/*
Get the value of a global HTTP request header.

Usage:
header = unleashed\http::getHeader("Content-type");
*/
getHeader(key) {
	key = toLower(key);
	if (!isDefined(level._unleashed)) {
		level._unleashed = [];
	}
	if (!isDefined(level._unleashed["http_headers"])) {
		level._unleashed["http_headers"] = [];
	}
	for (i = 0; i < level._unleashed["http_headers"].size; i++) {
		header = strTok(level._unleashed["http_headers"][i], ": ");
		if (header[0] == key) {
			return header[1];
		}
	}
	return undefined;
}

/*
Appends the http_base_path, if one is set, to the input argument.

Usage:
path = unleashed\http::path("/users");
*/
path(url) {
	if (isDefined(level._unleashed["http_base_path"])) {
		return level._unleashed["http_base_path"] + url;
	}
	return url;
}

/*
Performs an HTTP GET request and returns the response.

Usage:
response = unleashed\http::get("/users");
*/
get(url, headers) {
	return request("GET", url, undefined, headers);
}

/*
Performs an HTTP POST request and returns the response.

Usage:
response = unleashed\http::post("/users", user);
*/
post(url, body, headers) {
	return request("POST", url, body, headers);
}

/*
Performs an HTTP PUT request and returns the response.

Usage:
response = unleashed\http::put("/users/" + id, user);
*/
put(url, body, headers) {
	return request("PUT", url, body, headers);
}

/*
Performs an HTTP PATCH request and returns the response.

Usage:
response = unleashed\http::patch("/users/" + id, user);
*/
patch(url, body, headers) {
	return request("PATCH", url, body, headers);
}

/*
Performs an HTTP DELETE request and returns the response.

Usage:
response = unleashed\http::_delete("/users/" + id);
*/
_delete(url, body, headers) {
	return request("DELETE", url, body, headers);
}

/*
Performs an HTTP request and returns the response.

Usage:
response = unleashed\http::request("GET", "/users");
*/
request(method, url, body, headers) {
	requestHeaders = [];
	if (isDefined(level._unleashed["http_headers"])) {
		for (i = 0; i < level._unleashed["http_headers"].size; i++) {
			requestHeaders[requestHeaders.size] = level._unleashed["http_headers"][i];
		}
	}
	if (isDefined(headers)) {
		keys = getArrayKeys(headers);
		for (i = 0; i < keys.size; i++) {
			requestHeaders[requestHeaders.size] = keys[i] + ": " + headers[keys[i]];
		}
	}

	return httpRequest(
		method,
		path(url),
		body,
		requestHeaders[0],
		requestHeaders[1],
		requestHeaders[2],
		requestHeaders[3],
		requestHeaders[4],
		requestHeaders[5],
		requestHeaders[6],
		requestHeaders[7],
		requestHeaders[8],
		requestHeaders[9]
	);
}

/*
Create an HTTP request object.

Usage:
request = unleashed\http::http();
*/
http() {
	struct = spawnStruct();
	struct._unleashedHttp = true;
	return struct;
}

/*
Sets the method of an HTTP request object.

Usage:
request unleashed\http::setMethod("POST");
*/
setMethod(method) {
	self.method = method;
}

/*
Sets the url of an HTTP request object.

Usage:
request unleashed\http::setUrl("/users");
*/
setUrl(url) {
	self.url = url;
}

/*
Sets the body of an HTTP request object.

Usage:
request unleashed\http::setBody(user);
*/
setBody(body) {
	self.body = body;
}

/*
Send an HTTP request using an HTTP request object.

Usage:
request unleashed\http::send();
*/
send() {
	assertEx(isDefined(self), "trying to send request with uninitialised request object");
	assertEx(isDefined(self.url), "request URL not defined");
	assertEx(isDefined(self.method), "request method not defined");
	assertEx(!isDefined(self.body) || isString(self.body), "request body must be a string");
	assertEx(!isDefined(self.headers) || isArray(self.headers), "request headers must be an array");

	return request(
		self.method,
		self.url,
		self.body,
		self.headers
	);
}
