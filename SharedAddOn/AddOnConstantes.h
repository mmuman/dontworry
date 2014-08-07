/***************************/
/* Constantes des messages */
/***************************/

#ifndef _ADDONCONSTANTES_H
#define _ADDONCONSTANTES_H

// identifiant de messages
#define		ADD_DATAS_LIST				'Adtl'
#define		ADD_DATAS_TEXT				'Adtt'
#define		ADD_COMPLETION_DATAS		'Acod'
#define		ADD_FILE_MSG				'AddM'
#define		CALL_COMPTETE_WORD			'Ccow'
#define		CLOSE_WINDOW_MSG			'Cwfi'
#define		DISPLAY_BUFFER_MSG			'Dbum'
#define		DISPLAY_SEARCH_RESULT_MSG	'Dsrt'
#define		LAUNCH_SEARCH_MSG			'Lsmg'
#define		LOCK_MOTHER_WIN				'Lomw'
#define		MOVE_WINDOW_PTR				'Mwpt'
#define		NEW_FILE_MSG				0x2
#define		OPEN_FILE_MSG				'OpFi'
#define		OPEN_PROJECT_FILE			'Oppr'
#define		PREFS_CHANGED_MSG			'Pchm'
#define		REMOVE_FILE_MSG				'RemM'
#define		REFRESH_SYSLIB_MSG			'Rslm'
#define		REFRESH_HEADER_MSG			'Rrhd'
#define		UNLOCK_MOTHER_WIN			'Ulmw'
#define		UPDATE_FILE_MSG				'UpFi'


// vue "progress" ou fenetre de progression 
#define		PROGRESS_START_MSG			'Pstm'
#define		PROGRESS_STOP_MSG			'PSpm'
#define		PROGRESS_INCREASE_MSG		'Spaw'

// noms
#define		SHOW_WINDOW_RECT	"show-win-rect"
#define		VARIABLE_NAME		"variable-name"
#define		CLASSE_NAME			"classe-name"
#define		MOTHER_VIEW			"view-target"
#define		SYSTEM_PATH			"system-path"
#define		PROJECT_PATH		"project-path"
#define		SRING_TO_ADD		"string-to-add"
#define		DISPLAY_BUFFER		"display-buffer"
#define		PROJECT_FILE_NAME	"proj-file-name"
#define		PROJECT_FILE_PATH	"proj-file-path"
#define		FILE_NAME			"file-name"
#define		FILE_PATHNAME		"file-path-name"
#define		NB_DATAS_TO_ADD		"nb-datas-to-add"
#define		SELECTION			"selection"
#define		PROJECT_FILE_TOTAL	"nb-file-project"
#define		WINDOW_RESULT_PTR	"win-res-ptr"
#define		HEADER_TEXT			"header-text"
#define		RECURS_LEVEL		"recurs-level"
#define		VARIABLE_LOCAL		"variable-local"

// definition pour les donnees
#define		CS_ASK_TYPE			"CS-ask-type"
#define		CS_TYPE				"CS-Type"
#define		CS_RETURN			"CS-Return"
#define		CS_NAME				"CS-Name"
#define		CS_PARAMETER		"CS-Parameter"
#define		CS_PROTECTION		"CS-protection"
#define		CS_VIRTUAL			"CS-virtual"
#define		TYPE_CLASS			0
#define		TYPE_METHODE		1
#define		TYPE_VARIABLE		2
#define		TYPE_UNDIFINED		-1

// savoir ce que l'on demande
#define		ASK_POINTER			0	// "->"
#define		ASK_OBJECT			1	// "."
#define		ASK_METHOD			2	// "::"
#define		ASK_COMPLETION		3	// "completion"
#define		ASK_HELP_BEHAPPY	4	// "aide sur fonction"

// controle de BeHappy
#define		BH_ADDON_SHOW			'Bhas'
#define		BH_ADDON_NAME			"Be Book"
#define		BH_ADDON_FILE_NAME		"BeBook"

// chemin
#define		SETTING_PATH		"AddOnBeIDE"
#define		IMAGES_PATH			"Images"

// nom de la vue AddOn que l'on ajoute a chaque fenetre
#define		ADDON_VIEW_NAME		"DontWorryAddOnView"
#define		ADDON_NAME			"DontWorry"

// pour la vue icon (pour la faire clignoter)
#define		BLINK_ICONVIEW_MSG	'Bicm'

// constantes sur l'ajout de texte
#define		NO_ADD_TEXT_OPTION			0x0
#define		ADD_ENDLINE_OPTION			0x1
#define		PLACE_ON_END_LINE_OPTION	0x2

#endif _ADDONCONSTANTES_H