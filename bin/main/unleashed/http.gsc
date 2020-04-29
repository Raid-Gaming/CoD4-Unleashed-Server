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
Sets a header for use with all HTTP requests.

Usage:
unleashed\http::setHeader("Content-type", "application/json");
*/
setHeader(key, value) {
	key = toLower(key);
	if (!isDefined(level._unleashed)) {
		level._unleashed = [];
	}
	if (!isDefined(level._unleashed["http_headers"])) {
		level._unleashed["http_headers"] = [];
	}

	index = level._unleashed["http_headers"].size;
	for (i = 0; i < level._unleashed["http_headers"].size; i++) {
		if (level._unleashed["http_headers"][i] == key) {
			index = i;
			break;
		}
	}
	level._unleashed[index] = "" + key + ": " + value;
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
	return level._unleashed["http_headers"][key];
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
		for (i = 0; i < headers.size; i++) {
			requestHeaders[requestHeaders.size] = headers[i];
		}
	}

	return _performRequest(
		method,
		url,
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

_performRequest(method, url, body, header1, header2, header3, header4, header5, header6, header7, header8, header9, header10) {
	return httpRequest(
		method,
		path(url),
		body,
		header1,
		header2,
		header3,
		header4,
		header5,
		header6,
		header7,
		header8,
		header9,
		header10
	);
}
