// FBM utility
// ----------------------------------
//
// (c) 2004 José Luis Cebrián 
//
// Conversion utility for FBM files. Use:
//
//	FBM file.fbm file.png		; Conversion from FBM to PNG
//	FBM file.png file.fbm		; Conversion to FBM
//	FBM file.fbm         		; Show information about FBM
//
// Also, the following commands can be append:
//
//	CENTER:x,y			; Set the graphic center
//	n:x,y				; Set the control point n

import "Image";

global
	string	destination;
	string	source;
	string	commands[99];
	int	command_count;
end

// ---------------------------------------------------------------------

function Syntax()
begin
	say ("Use: FBM source [destination]");
end

// ---------------------------------------------------------------------

process Main()
private
	int i, pos;
	string command, parameters;
begin
	say ("FBM Utility - Copyright (c) 2004 Fenix Team");
	say ("FBM comes with ABSOLUTELY NO WARRANTY; see COPYING for details");
	say ("");

	for (i = 1 ; i <= argc ; i++)
		if (regex(":", argv[i]) != -1)
			commands[command_count++] = argv[i];
			if (command_count == 100)
				say ("Too many commands!");
			end
		elseif (source == "")
			source = argv[i];
		elseif (destination == "")
			destination = argv[i];
		else
			Syntax();
			exit();
		end
	end

	// Nothing to do? 

	if (source == "")
		Syntax();
		exit();
	end

	// -----------------------------------------------------------------
	//  Load the file from disk
	// -----------------------------------------------------------------

	// Load the original file

	if (regex("\.fbm$", source) != -1)
		graph = load_fbm(source);
	elseif (regex("\.map$", source) != -1)
		graph = load_map(source);
	elseif (regex("\.png$", source) != -1)
		graph = load_png(source);
		if (graph < 0)
			say ("Error al cargar " + source);
			exit(0);
		end
		if (destination == "")
			destination = regex_replace("\.png$", ".fbm", source);
		end
	else
		// Nothing to load

		graph = load_image(source);
		if (destination == "")
			destination = regex_replace("\.*$", ".fbm", source);
		end
	end

	// -----------------------------------------------------------------
	//  Execute command line options
	// -----------------------------------------------------------------

	for (i = 0 ; i < command_count ; i++)
		pos = regex(":", commands[i]);
		command = substr(commands[i], 0, pos);
		parameters = substr(commands[i], pos+1);
		pos = regex(",", parameters);
		if (pos != -1)
			x = (int)substr(parameters, 0, pos);
			y = (int)substr(parameters, pos+1);
		end

		if (command == "center")
			set_center(0, graph, x, y);
		elseif (command == "name")
			map_set_name(0, graph, parameters);
		elseif ((int)command > 0)
			set_point (0, graph, (int)command, x, y);
		else
			say ("Command " + command + " unknown");
			exit(0);
		end
	end

	// -----------------------------------------------------------------
	//  Save the file to disk
	// -----------------------------------------------------------------

	if (destination == "" or command_count > 0)
	
		// Show information about the file

		say (ucase(source));
		say ("");
		say ("Name      : " + map_name(0, graph));
		say ("Size      : " + graphic_info(0, graph, G_WIDTH) +
			   " x " + graphic_info(0, graph, G_HEIGHT));
		say ("Depth     : " + graphic_info(0, graph, G_DEPTH) + " bpp");
		say ("Center    : " + graphic_info(0, graph, G_CENTER_X) +
			   ", " + graphic_info(0, graph, G_CENTER_Y));

		for (i = 1 ; i < 1000 ; i++)
			if (get_point(0, graph, i, &x, &y))
				say ("Point " + lpad(i, 3)+ " : " + x + ", " + y);
			end
		end

		if (command_count > 0 && destination == "")
			save_fbm (0, graph, source);
			say ("");
			say (source + " (Saved)");
		elseif (destination != "")
			say ("");
		end
	end

	if (destination != "")

		// Convert the file to another format

		if (regex("\.png$", destination) != -1)
			save_png(0, graph, destination);
		elseif (regex("\.fbm$", destination) != -1)
			save_fbm(0, graph, destination);
		else
			say (destination + ": unknown extension");
			exit();
		end

		say (source + " -> " + destination + " (Saved)");
	end
end
