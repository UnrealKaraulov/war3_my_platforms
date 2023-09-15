#include "httplib.h"

int main()
{
	using namespace httplib;
	Server httpMapUploaderSever;

	httpMapUploaderSever.Get("/", [ ](const Request& req, Response& res) {
		res.set_content("<form enctype=\"multipart/form-data\" method=\"post\" action=\"/uploadmap\"><input type=\"file\" name=\"file\" accept=\".w3x,.w3m\" value=\"Select map file\"><input type=\"submit\" name=\"submit\" value=\"Upload map\"></form>", "text/html");
	});

	httpMapUploaderSever.Get("/rus", [ ](const Request& req, Response& res) {
		res.set_content("empty", "text/html");
	});

	httpMapUploaderSever.Get("/maplist", [ ](const Request& req, Response& res) {
		res.set_content("empty", "text/html");
	});

	httpMapUploaderSever.Get("/test", [ ](const Request& req, Response& res) {
		res.set_content("empty", "text/html");
	});

	httpMapUploaderSever.Get("/viewmap", [ ](const Request& req, Response& res) {
		res.set_content("empty", "text/html");
	});

	httpMapUploaderSever.Get("/delete", [ ](const Request& req, Response& res) {
		res.set_content("empty", "text/html");
	});

	httpMapUploaderSever.Get("/download", [ & ](const Request& req, Response& res) {
		res.set_content("empty", "text/html");
	});

	httpMapUploaderSever.Post("/uploadmap", [ & ](const Request& req, Response& res) {
		if (req.is_multipart_form_data())
		{
			res.set_content("SUCCESS", "text/html");
		}
	});

	httpMapUploaderSever.listen("0.0.0.0", 80);
}