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
- http_content_type

Usage:
unleashed\http::setOpt("http_base_path", "http://localhost");
*/
setOpt(key, value) {
	if (!isDefined(level._unleashed)) {
		level._unleashed = [];
	}
	level._unleashed[key] = value;
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
Return the http_content_type option or the default content-type (application/json)
if no argument is provided.

Mainly intended for internal use within the provided http wrapper functions.

Usage:
contentType = unleashed\http::getContentType();
*/
getContentType(contentType) {
	if (isDefined(contentType)) {
		return contentType;
	}
	if (isDefined(level._unleashed["http_content_type"])) {
		return level._unleashed["http_content_type"];
	}
	return "application/json";
}

/*
Performs an HTTP GET request and returns the response.

Usage:
response = unleashed\http::get("/users");
*/
get(url) {
	return httpGet(path(url));
}

/*
Performs an HTTP POST request and returns the response.

Usage:
response = unleashed\http::post("/users", user);
*/
post(url, body, contentType) {
	return httpPost(
		path(url),
		body,
		getContentType(contentType)
	);
}

/*
Performs an HTTP PUT request and returns the response.

Usage:
response = unleashed\http::put("/users/" + id, user);
*/
put(url, body, contentType) {
	return httpPut(
		path(url),
		body,
		getContentType(contentType)
	);
}

/*
Performs an HTTP PATCH request and returns the response.

Usage:
response = unleashed\http::patch("/users/" + id, user);
*/
patch(url, body, contentType) {
	return httpPatch(
		path(url),
		body,
		getContentType(contentType)
	);
}

/*
Performs an HTTP DELETE request and returns the response.

Usage:
response = unleashed\http::_delete("/users/" + id);
*/
_delete(url, body, contentType) {
	if (!isDefined(body)) {
		return httpDelete(path(url));
	}
	return httpDelete(
		path(url),
		body,
		getContentType(contentType)
	);
}

/*
Performs an HTTP request and returns the response.

Usage:
response = unleashed\http::request("GET", "/users");
*/
request(method, url, body, contentType) {
	if (!isDefined(body)) {
		return httpRequest(
			method,
			path(url)
		);
	}
	return httpRequest(
		method,
		path(url),
		body,
		getContentType(contentType)
	);
}
