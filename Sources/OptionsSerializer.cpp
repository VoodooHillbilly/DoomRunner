//======================================================================================================================
// Project: DoomRunner
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: serialization/deserialization of the user data into/from a file
//======================================================================================================================

#include "OptionsSerializer.hpp"

#include "JsonUtils.hpp"
#include "MiscUtils.hpp"  // checkPath
#include "Version.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>


const QString defaultOptionsFileName = "options.json";


//======================================================================================================================
//  serialization utils

template< typename Elem >
QJsonArray serializeList( const QList< Elem > & list )
{
	QJsonArray jsArray;
	for (const Elem & elem : list)
	{
		jsArray.append( serialize( elem ) );
	}
	return jsArray;
}

QJsonArray serializeStringList( const QList< QString > & list )
{
	QJsonArray jsArray;
	for (const auto & elem : list)
	{
		jsArray.append( elem );
	}
	return jsArray;
}

void deserializeStringList( const JsonArrayCtx & jsList, QList< QString > & list )
{
	for (int i = 0; i < jsList.size(); i++)
	{
		QString elem = jsList.getString( i );
		if (!elem.isEmpty())
			list.append( std::move(elem) );
	}
}


//======================================================================================================================
//  user data serialization

static QJsonObject serialize( const Engine & engine )
{
	QJsonObject jsEngine;

	if (engine.isSeparator)
	{
		jsEngine["separator"] = true;
		jsEngine["name"] = engine.name;
	}
	else
	{
		jsEngine["name"] = engine.name;
		jsEngine["path"] = engine.path;
		jsEngine["config_dir"] = engine.configDir;
		jsEngine["family"] = familyToStr( engine.family );
	}

	return jsEngine;
}

static void deserialize( const JsonObjectCtx & jsEngine, Engine & engine )
{
	engine.isSeparator = jsEngine.getBool( "separator", engine.isSeparator, DontShowError );
	if (engine.isSeparator)
	{
		engine.name = jsEngine.getString( "name", "<missing name>" );
	}
	else
	{
		engine.name = jsEngine.getString( "name", "<missing name>" );
		engine.path = jsEngine.getString( "path", "" );  // empty path is used to indicate invalid entry to be skipped
		engine.configDir = jsEngine.getString( "config_dir", getDirOfFile( engine.path ) );
		engine.family = familyFromStr( jsEngine.getString( "family", "", DontShowError ) );
		if (engine.family == EngineFamily::_EnumEnd)
			engine.family = guessEngineFamily( getFileBasenameFromPath( engine.path ) );
	}
}

static QJsonObject serialize( const IWAD & iwad )
{
	QJsonObject jsIWAD;

	if (iwad.isSeparator)
	{
		jsIWAD["separator"] = true;
		jsIWAD["name"] = iwad.name;
	}
	else
	{
		jsIWAD["name"] = iwad.name;
		jsIWAD["path"] = iwad.path;
	}

	return jsIWAD;
}

static void deserialize( const JsonObjectCtx & jsEngine, IWAD & iwad )
{
	iwad.isSeparator = jsEngine.getBool( "separator", iwad.isSeparator, DontShowError );
	if (iwad.isSeparator)
	{
		iwad.name = jsEngine.getString( "name", "<missing name>" );
	}
	else
	{
		iwad.path = jsEngine.getString( "path", "" );  // empty path is used to indicate invalid entry to be skipped
		iwad.name = jsEngine.getString( "name", QFileInfo( iwad.path ).fileName() );
	}
}

static QJsonObject serialize( const Mod & mod )
{
	QJsonObject jsMod;

	if (mod.isSeparator)
	{
		jsMod["separator"] = true;
		jsMod["name"] = mod.fileName;
	}
	else
	{
		jsMod["path"] = mod.path;
		jsMod["checked"] = mod.checked;
	}

	return jsMod;
}

static void deserialize( const JsonObjectCtx & jsMod, Mod & mod )
{
	mod.isSeparator = jsMod.getBool( "separator", mod.isSeparator, DontShowError );
	if (mod.isSeparator)
	{
		mod.fileName = jsMod.getString( "name", "<missing name>" );
	}
	else
	{
		mod.path = jsMod.getString( "path", "" );  // empty path is used to indicate invalid entry to be skipped
		mod.fileName = QFileInfo( mod.path ).fileName();
		mod.checked = jsMod.getBool( "checked", mod.checked );
	}
}

static QJsonObject serialize( const IwadSettings & iwadSettings )
{
	QJsonObject jsIWADs;

	jsIWADs["auto_update"] = iwadSettings.updateFromDir;
	jsIWADs["directory"] = iwadSettings.dir;
	jsIWADs["search_subdirs"] = iwadSettings.searchSubdirs;

	return jsIWADs;
}

static void deserialize( const JsonObjectCtx & jsIWADs, IwadSettings & iwadSettings )
{
	iwadSettings.updateFromDir = jsIWADs.getBool( "auto_update", iwadSettings.updateFromDir );
	iwadSettings.dir = jsIWADs.getString( "directory" );
	iwadSettings.searchSubdirs = jsIWADs.getBool( "search_subdirs", iwadSettings.searchSubdirs );
}

static QJsonObject serialize( const MapSettings & mapSettings )
{
	QJsonObject jsMaps;

	jsMaps["directory"] = mapSettings.dir;

	return jsMaps;
}

static void deserialize( const JsonObjectCtx & jsMaps, MapSettings & mapSettings )
{
	mapSettings.dir = jsMaps.getString( "directory" );
}

static QJsonObject serialize( const ModSettings & modSettings )
{
	QJsonObject jsMods;

	jsMods["directory"] = modSettings.dir;

	return jsMods;
}

static void deserialize( const JsonObjectCtx & jsMods, ModSettings & modSettings )
{
	modSettings.dir = jsMods.getString( "directory" );
}

static QJsonObject serialize( const LaunchOptions & opts )
{
	QJsonObject jsOptions;

	jsOptions["launch_mode"] = int( opts.mode );
	jsOptions["map_name"] = opts.mapName;
	jsOptions["save_file"] = opts.saveFile;
	jsOptions["map_name_demo"] = opts.mapName_demo;
	jsOptions["demo_file_record"] = opts.demoFile_record;
	jsOptions["demo_file_replay"] = opts.demoFile_replay;

	return jsOptions;
}

static void deserialize( const JsonObjectCtx & jsOptions, LaunchOptions & opts )
{
	opts.mode = jsOptions.getEnum< LaunchMode >( "launch_mode", opts.mode );
	opts.mapName = jsOptions.getString( "map_name", opts.mapName );
	opts.saveFile = jsOptions.getString( "save_file", opts.saveFile );
	opts.mapName_demo = jsOptions.getString( "map_name_demo", opts.mapName_demo );
	opts.demoFile_record = jsOptions.getString( "demo_file_record", opts.demoFile_record );
	opts.demoFile_replay = jsOptions.getString( "demo_file_replay", opts.demoFile_replay );
}

static QJsonObject serialize( const MultiplayerOptions & opts )
{
	QJsonObject jsOptions;

	jsOptions["is_multiplayer"] = opts.isMultiplayer;
	jsOptions["mult_role"] = int( opts.multRole );
	jsOptions["host_name"] = opts.hostName;
	jsOptions["port"] = int( opts.port );
	jsOptions["net_mode"] = int( opts.netMode );
	jsOptions["game_mode"] = int( opts.gameMode );
	jsOptions["player_count"] = int( opts.playerCount );
	jsOptions["team_damage"] = opts.teamDamage;
	jsOptions["time_limit"] = int( opts.timeLimit );
	jsOptions["frag_limit"] = int( opts.fragLimit );

	return jsOptions;
}

static void deserialize( const JsonObjectCtx & jsOptions, MultiplayerOptions & opts )
{
	opts.isMultiplayer = jsOptions.getBool( "is_multiplayer", opts.isMultiplayer );
	opts.multRole = jsOptions.getEnum< MultRole >( "mult_role", opts.multRole );
	opts.hostName = jsOptions.getString( "host_name", opts.hostName );
	opts.port = jsOptions.getUInt16( "port", opts.port );
	opts.netMode = jsOptions.getEnum< NetMode >( "net_mode", opts.netMode );
	opts.gameMode = jsOptions.getEnum< GameMode >( "game_mode", opts.gameMode );
	opts.playerCount = jsOptions.getUInt( "player_count", opts.playerCount );
	opts.teamDamage = jsOptions.getDouble( "team_damage", opts.teamDamage );
	opts.timeLimit = jsOptions.getUInt( "time_limit", opts.timeLimit );
	opts.fragLimit = jsOptions.getUInt( "frag_limit", opts.fragLimit );
}

static QJsonObject serialize( const GameplayOptions & opts )
{
	QJsonObject jsOptions;

	jsOptions["skill_num"] = int( opts.skillNum );
	jsOptions["no_monsters"] = opts.noMonsters;
	jsOptions["fast_monsters"] = opts.fastMonsters;
	jsOptions["monsters_respawn"] = opts.monstersRespawn;
	jsOptions["dmflags1"] = qint64( opts.details.flags1 );
	jsOptions["dmflags2"] = qint64( opts.details.flags2 );
	jsOptions["allow_cheats"] = opts.allowCheats;

	return jsOptions;
}

static void deserialize( const JsonObjectCtx & jsOptions, GameplayOptions & opts )
{
	opts.skillNum = jsOptions.getInt( "skill_num", opts.skillNum );
	opts.noMonsters = jsOptions.getBool( "no_monsters", opts.noMonsters );
	opts.fastMonsters = jsOptions.getBool( "fast_monsters", opts.fastMonsters );
	opts.monstersRespawn = jsOptions.getBool( "monsters_respawn", opts.monstersRespawn );
	opts.details.flags1 = jsOptions.getInt( "dmflags1", opts.details.flags1 );
	opts.details.flags2 = jsOptions.getInt( "dmflags2", opts.details.flags2 );
	opts.allowCheats = jsOptions.getBool( "allow_cheats", opts.allowCheats );
}

static QJsonObject serialize( const CompatibilityOptions & opts )
{
	QJsonObject jsOptions;

	jsOptions["compat_level"] = opts.level;
	jsOptions["compatflags1"] = qint64( opts.details.flags1 );
	jsOptions["compatflags2"] = qint64( opts.details.flags2 );

	return jsOptions;
}

static void deserialize( const JsonObjectCtx & jsOptions, CompatibilityOptions & opts )
{
	opts.details.flags1 = jsOptions.getInt( "compatflags1", opts.details.flags1 );
	opts.details.flags2 = jsOptions.getInt( "compatflags2", opts.details.flags2 );
	opts.level = jsOptions.getInt( "compat_level", opts.level, DontShowError );
}

static QJsonObject serialize( const AlternativePaths & opts )
{
	QJsonObject jsOptions;

	jsOptions["save_dir"] = opts.saveDir;
	jsOptions["screenshot_dir"] = opts.screenshotDir;

	return jsOptions;
}

static void deserialize( const JsonObjectCtx & jsOptions, AlternativePaths & opts )
{
	opts.saveDir = jsOptions.getString( "save_dir", opts.saveDir );
	opts.screenshotDir = jsOptions.getString( "screenshot_dir", opts.screenshotDir );
}

static QJsonObject serialize( const VideoOptions & opts )
{
	QJsonObject jsOptions;

	jsOptions["monitor_idx"] = opts.monitorIdx;
	jsOptions["resolution_x"] = qint64( opts.resolutionX );
	jsOptions["resolution_y"] = qint64( opts.resolutionY );
	jsOptions["show_fps"] = opts.showFPS;

	return jsOptions;
}

static void deserialize( const JsonObjectCtx & jsOptions, VideoOptions & opts )
{
	opts.monitorIdx = jsOptions.getInt( "monitor_idx", opts.monitorIdx );
	opts.resolutionX = jsOptions.getUInt( "resolution_x", opts.resolutionX );
	opts.resolutionY = jsOptions.getUInt( "resolution_y", opts.resolutionY );
	opts.showFPS = jsOptions.getBool( "show_fps", opts.showFPS, DontShowError );
}

static QJsonObject serialize( const AudioOptions & opts )
{
	QJsonObject jsOptions;

	jsOptions["no_sound"] = opts.noSound;
	jsOptions["no_sfx"] = opts.noSFX;
	jsOptions["no_music"] = opts.noMusic;

	return jsOptions;
}

static void deserialize( const JsonObjectCtx & jsOptions, AudioOptions & opts )
{
	opts.noSound = jsOptions.getBool( "no_sound", opts.noSound );
	opts.noSFX = jsOptions.getBool( "no_sfx", opts.noSFX );
	opts.noMusic = jsOptions.getBool( "no_music", opts.noMusic );
}

static QJsonObject serialize( const GlobalOptions & opts )
{
	QJsonObject jsOptions;

	jsOptions["use_preset_name_as_dir"] = opts.usePresetNameAsDir;
	jsOptions["additional_args"] = opts.cmdArgs;

	return jsOptions;
}

static void deserialize( const JsonObjectCtx & jsOptions, GlobalOptions & opts )
{
	opts.usePresetNameAsDir = jsOptions.getBool( "use_preset_name_as_dir", opts.usePresetNameAsDir, DontShowError );
	opts.cmdArgs = jsOptions.getString( "additional_args" );
}

static QJsonObject serialize( const Preset & preset, const StorageSettings & settings )
{
	QJsonObject jsPreset;

	jsPreset["name"] = preset.name;

	if (preset.isSeparator)
	{
		jsPreset["separator"] = true;
		return jsPreset;
	}

	// files

	jsPreset["selected_engine"] = preset.selectedEnginePath;
	jsPreset["selected_config"] = preset.selectedConfig;
	jsPreset["selected_IWAD"] = preset.selectedIWAD;

	jsPreset["selected_mappacks"] = serializeStringList( preset.selectedMapPacks );

	jsPreset["mods"] = serializeList( preset.mods );

	// options

	if (settings.launchOptsStorage == StoreToPreset)
		jsPreset["launch_options"] = serialize( preset.launchOpts );

	if (settings.launchOptsStorage == StoreToPreset)
		jsPreset["multiplayer_options"] = serialize( preset.multOpts );

	if (settings.gameOptsStorage == StoreToPreset)
		jsPreset["gameplay_options"] = serialize( preset.gameOpts );

	if (settings.compatOptsStorage == StoreToPreset)
		jsPreset["compatibility_options"] = serialize( preset.compatOpts );

	jsPreset["alternative_paths"] = serialize( preset.altPaths );

	// preset-specific args

	jsPreset["additional_args"] = preset.cmdArgs;

	return jsPreset;
}

static void deserialize( const JsonObjectCtx & jsPreset, Preset & preset, const StorageSettings & settings )
{
	preset.name = jsPreset.getString( "name", "<missing name>" );

	preset.isSeparator = jsPreset.getBool( "separator", preset.isSeparator, DontShowError );
	if (preset.isSeparator)
	{
		return;
	}

	// files

	preset.selectedEnginePath = jsPreset.getString( "selected_engine" );
	preset.selectedConfig = jsPreset.getString( "selected_config" );
	preset.selectedIWAD = jsPreset.getString( "selected_IWAD" );

	if (JsonArrayCtx jsSelectedMapPacks = jsPreset.getArray( "selected_mappacks" ))
	{
		deserializeStringList( jsSelectedMapPacks, preset.selectedMapPacks );
	}

	if (JsonArrayCtx jsMods = jsPreset.getArray( "mods" ))
	{
		// iterate manually, so that we can filter-out invalid items
		for (int i = 0; i < jsMods.size(); i++)
		{
			JsonObjectCtx jsMod = jsMods.getObject( i );
			if (!jsMod)  // wrong type on position i - skip this entry
				continue;

			Mod mod;
			deserialize( jsMod, mod );

			if (mod.isSeparator && mod.fileName.isEmpty())  // element isn't present in JSON -> skip this entry
				continue;
			else if (!mod.isSeparator && mod.path.isEmpty())  // element isn't present in JSON -> skip this entry
				continue;

			// check path or not?

			preset.mods.append( std::move( mod ) );
		}
	}

	// options

	if (settings.launchOptsStorage == StoreToPreset)
		if (JsonObjectCtx jsOptions = jsPreset.getObject( "launch_options" ))
			deserialize( jsOptions, preset.launchOpts );

	if (settings.launchOptsStorage == StoreToPreset)
		if (JsonObjectCtx jsOptions = jsPreset.getObject( "multiplayer_options" ))
			deserialize( jsOptions, preset.multOpts );

	if (settings.gameOptsStorage == StoreToPreset)
		if (JsonObjectCtx jsOptions = jsPreset.getObject( "gameplay_options" ))
			deserialize( jsOptions, preset.gameOpts );

	if (settings.compatOptsStorage == StoreToPreset)
		if (JsonObjectCtx jsOptions = jsPreset.getObject( "compatibility_options" ))
			deserialize( jsOptions, preset.compatOpts );

	if (JsonObjectCtx jsOptions = jsPreset.getObject( "alternative_paths" ))
		deserialize( jsOptions, preset.altPaths );

	// preset-specific args

	preset.cmdArgs = jsPreset.getString( "additional_args" );
}

static QJsonObject serialize( const WindowGeometry & geometry )
{
	QJsonObject jsGeometry;

	jsGeometry["width"] = geometry.width;
	jsGeometry["height"] = geometry.height;

	return jsGeometry;
}

static void deserialize( const JsonObjectCtx & jsGeometry, WindowGeometry & geometry )
{
	geometry.width = jsGeometry.getInt( "width", -1 );
	geometry.height = jsGeometry.getInt( "height", -1 );
}

static void serialize( QJsonObject & jsSettings, const LauncherSettings & settings )
{
	jsSettings["check_for_updates"] = settings.checkForUpdates;
	jsSettings["use_absolute_paths"] = settings.useAbsolutePaths;
	jsSettings["close_on_launch"] = settings.closeOnLaunch;
	jsSettings["show_engine_output"] = settings.showEngineOutput;

	{
		QJsonObject jsOptsStorage;

		jsOptsStorage["launch_opts"] = int( settings.launchOptsStorage );
		jsOptsStorage["gameplay_opts"] = int( settings.gameOptsStorage );
		jsOptsStorage["compat_opts"] = int( settings.compatOptsStorage );

		jsSettings["options_storage"] = jsOptsStorage;
	}
}

static void deserialize( const JsonObjectCtx & jsSettings, LauncherSettings & settings )
{
	settings.checkForUpdates = jsSettings.getBool( "check_for_updates", settings.checkForUpdates, DontShowError );
	settings.useAbsolutePaths = jsSettings.getBool( "use_absolute_paths", settings.useAbsolutePaths );
	settings.closeOnLaunch = jsSettings.getBool( "close_on_launch", settings.closeOnLaunch, DontShowError );
	settings.showEngineOutput = jsSettings.getBool( "show_engine_output", settings.showEngineOutput, DontShowError );

	if (JsonObjectCtx jsOptsStorage = jsSettings.getObject( "options_storage" ))
	{
		settings.launchOptsStorage = jsOptsStorage.getEnum< OptionsStorage >( "launch_opts", settings.launchOptsStorage );
		settings.gameOptsStorage = jsOptsStorage.getEnum< OptionsStorage >( "gameplay_opts", settings.gameOptsStorage );
		settings.compatOptsStorage = jsOptsStorage.getEnum< OptionsStorage >( "compat_opts", settings.compatOptsStorage );
	}
}


//======================================================================================================================
//  options file stucture

static void serializeOptionsToJson( const OptionsToSave & opts, QJsonObject & jsOpts )
{
	// files and related settings

	{
		// better keep room for adding some engine settings later, so that we don't have to break compatibility again
		QJsonObject jsEngines;

		jsEngines["engine_list"] = serializeList( opts.engines );

		jsOpts["engines"] = jsEngines;
	}

	{
		QJsonObject jsIWADs = serialize( opts.iwadSettings );

		if (!opts.iwadSettings.updateFromDir)
			jsIWADs["IWAD_list"] = serializeList( opts.iwads );

		jsOpts["IWADs"] = jsIWADs;
	}

	jsOpts["maps"] = serialize( opts.mapSettings );

	jsOpts["mods"] = serialize( opts.modSettings );

	// options

	if (opts.settings.launchOptsStorage == StoreGlobally)
		jsOpts["launch_options"] = serialize( opts.launchOpts );

	if (opts.settings.launchOptsStorage == StoreGlobally)
		jsOpts["multiplayer_options"] = serialize( opts.multOpts );

	if (opts.settings.gameOptsStorage == StoreGlobally)
		jsOpts["gameplay_options"] = serialize( opts.gameOpts );

	if (opts.settings.compatOptsStorage == StoreGlobally)
		jsOpts["compatibility_options"] = serialize( opts.compatOpts );

	jsOpts["video_options"] = serialize( opts.videoOpts );

	jsOpts["audio_options"] = serialize( opts.audioOpts );

	jsOpts["global_options"] = serialize( opts.globalOpts );

	// presets

	{
		QJsonArray jsPresetArray;

		for (const Preset & preset : opts.presets)
		{
			QJsonObject jsPreset = serialize( preset, opts.settings );
			jsPresetArray.append( jsPreset );
		}

		jsOpts["presets"] = jsPresetArray;
	}

	jsOpts["selected_preset"] = opts.selectedPresetIdx >= 0 ? opts.presets[ opts.selectedPresetIdx ].name : "";

	// global settings

	serialize( jsOpts, opts.settings );

	jsOpts["geometry"] = serialize( opts.geometry );
}

static void deserializeOptionsFromJson( OptionsToLoad & opts, const JsonObjectCtx & jsOpts )
{
	// global settings

	// this must be loaded early, because we need to know whether to attempt loading the opts from the presets
	deserialize( jsOpts, opts.settings );

	if (JsonObjectCtx jsGeometry = jsOpts.getObject( "geometry" ))
	{
		deserialize( jsGeometry, opts.geometry );
	}

	// files and related settings

	if (JsonObjectCtx jsEngines = jsOpts.getObject( "engines" ))
	{
		if (JsonArrayCtx jsEngineArray = jsEngines.getArray( "engine_list" ))
		{
			// iterate manually, so that we can filter-out invalid items
			for (int i = 0; i < jsEngineArray.size(); i++)
			{
				JsonObjectCtx jsEngine = jsEngineArray.getObject( i );
				if (!jsEngine)  // wrong type on position i -> skip this entry
					continue;

				Engine engine;
				deserialize( jsEngine, engine );

				if (engine.path.isEmpty())  // element isn't present in JSON -> skip this entry
					continue;

				checkPath_MsgBox( engine.path, "An Engine from the saved options (%1) no longer exists. Please update it in Menu -> Setup." );

				opts.engines.append( std::move( engine ) );
			}
		}
	}

	if (JsonObjectCtx jsIWADs = jsOpts.getObject( "IWADs" ))
	{
		deserialize( jsIWADs, opts.iwadSettings );

		if (opts.iwadSettings.updateFromDir)
		{
			checkPath_MsgBox( opts.iwadSettings.dir, "IWAD directory from the saved options (%1) no longer exists. Please update it in Menu -> Setup." );
		}
		else
		{
			if (JsonArrayCtx jsIWADArray = jsIWADs.getArray( "IWAD_list" ))
			{
				// iterate manually, so that we can filter-out invalid items
				for (int i = 0; i < jsIWADArray.size(); i++)
				{
					JsonObjectCtx jsIWAD = jsIWADArray.getObject( i );
					if (!jsIWAD)  // wrong type on position i - skip this entry
						continue;

					IWAD iwad;
					deserialize( jsIWAD, iwad );

					if (iwad.name.isEmpty() || iwad.path.isEmpty())  // element isn't present in JSON -> skip this entry
						continue;

					checkPath_MsgBox( iwad.path, "An IWAD from the saved options (%1) no longer exists. Please update it in Menu -> Setup." );

					opts.iwads.append( std::move( iwad ) );
				}
			}
		}
	}

	if (JsonObjectCtx jsMaps = jsOpts.getObject( "maps" ))
	{
		deserialize( jsMaps, opts.mapSettings );

		checkPath_MsgBox( opts.mapSettings.dir, "Map directory from the saved options (%1) no longer exists. Please update it in Menu -> Setup." );
	}

	if (JsonObjectCtx jsMods = jsOpts.getObject( "mods" ))
	{
		deserialize( jsMods, opts.modSettings );

		checkPath_MsgBox( opts.modSettings.dir, "Mod directory from the saved options (%1) no longer exists. Please update it in Menu -> Setup." );
	}

	// options

	if (opts.settings.launchOptsStorage == StoreGlobally)
		if (JsonObjectCtx jsOptions = jsOpts.getObject( "launch_options" ))
			deserialize( jsOptions, opts.launchOpts );

	if (opts.settings.launchOptsStorage == StoreGlobally)
		if (JsonObjectCtx jsOptions = jsOpts.getObject( "multiplayer_options" ))
			deserialize( jsOptions, opts.multOpts );

	if (opts.settings.gameOptsStorage == StoreGlobally)
		if (JsonObjectCtx jsOptions = jsOpts.getObject( "gameplay_options" ))
			deserialize( jsOptions, opts.gameOpts );

	if (opts.settings.compatOptsStorage == StoreGlobally)
		if (JsonObjectCtx jsOptions = jsOpts.getObject( "compatibility_options" ))
			deserialize( jsOptions, opts.compatOpts );

	if (JsonObjectCtx jsOptions = jsOpts.getObject( "video_options" ))
		deserialize( jsOptions, opts.videoOpts );

	if (JsonObjectCtx jsOptions = jsOpts.getObject( "audio_options" ))
		deserialize( jsOptions, opts.audioOpts );

	if (JsonObjectCtx jsOptions = jsOpts.getObject( "global_options" ))
		deserialize( jsOptions, opts.globalOpts );

	// presets

	if (JsonArrayCtx jsPresetArray = jsOpts.getArray( "presets" ))
	{
		for (int i = 0; i < jsPresetArray.size(); i++)
		{
			JsonObjectCtx jsPreset = jsPresetArray.getObject( i );
			if (!jsPreset)  // wrong type on position i - skip this entry
				continue;

			Preset preset;
			deserialize( jsPreset, preset, opts.settings );

			opts.presets.append( std::move( preset ) );
		}
	}

	opts.selectedPreset = jsOpts.getString( "selected_preset" );
}


//======================================================================================================================
//  backward compatibility - loading user data from older options format

#include "OptionsSerializer_compat.cpp"  // hack, but it's ok in this case


//======================================================================================================================
//  JSON and version handling

static QByteArray serializeOptions( const OptionsToSave & opts )
{
	QJsonObject jsRoot;

	// this will be used to detect options created by older versions and supress "missing element" warnings
	jsRoot["version"] = appVersion;

	serializeOptionsToJson( opts, jsRoot );

	QJsonDocument jsonDoc( jsRoot );

	return jsonDoc.toJson();
}

static QString deserializeOptions( OptionsToLoad & opts, const QByteArray & bytes )
{
	QJsonParseError error;
	QJsonDocument jsonDoc = QJsonDocument::fromJson( bytes, &error );
	if (jsonDoc.isNull())
	{
		return "Failed to parse options.json: "%error.errorString()%"\n"
		       "You can either open it in notepad and try to repair it, or delete it and start from scratch.";
	}

	// We use this contextual mechanism instead of standard JSON getters, because when something fails to load
	// we want to print a useful error message with information exactly which JSON element is broken.
	JsonDocumentCtx jsonDocCtx( jsonDoc );
	const JsonObjectCtx & jsRoot = jsonDocCtx.rootObject();

	// backward compatibility with older options format
	QString optsVersion = jsRoot.getString( "version", "", DontShowError );
	if (optsVersion.isEmpty() || compareVersions( optsVersion, "1.7" ) < 0)
	{
		jsonDocCtx.disableWarnings();  // supress "missing element" warnings when loading older version

		// try to load as the 1.6.3 format, older versions will have to accept resetting some values to defaults
		deserializeOptionsFromJson_pre17( opts, jsRoot );
	}
	else
	{
		if (compareVersions( optsVersion, appVersion ) < 0)
			jsonDocCtx.disableWarnings();  // supress "missing element" warnings when loading older version

		deserializeOptionsFromJson( opts, jsRoot );
	}

	return {};
}


//======================================================================================================================
//  top-level API

QString writeOptionsToFile( const OptionsToSave & opts, const QString & filePath )
{
	QByteArray bytes = serializeOptions( opts );

	QString error = updateFileSafely( filePath, bytes );
	if (!error.isEmpty())
	{
		return error;
	}

	return {};
}

QString readOptionsFromFile( OptionsToLoad & opts, const QString & filePath )
{
	QByteArray bytes;
	QString error = readWholeFile( filePath, bytes );
	if (!error.isEmpty())
	{
		return error;
	}

	error = deserializeOptions( opts, bytes );
	if (!error.isEmpty())
	{
		return error;
	}

	return {};
}