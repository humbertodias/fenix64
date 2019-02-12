program fhello;
local
	fd;
begin
	fd = fopen("fhello.txt", 2);
	fputs(fd, "Hello world!");
end
