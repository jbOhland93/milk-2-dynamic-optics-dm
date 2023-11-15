#ifndef USERINPUTHANDLER_H
#define USERINPUTHANDLER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "../headers/dmController.h"
#include "../headers/ImageStreamManager.h"

enum class userCmd{
	CMD_HELP,
	CMD_ARM,
	CMD_UNARM,
	CMD_RELAX,
	CMD_QUIT
};

class UserInputHandler
{
    public:
		static std::vector<userCmd> cmdList;
		static std::map<std::string,userCmd> cmdStrings;
		static std::map<userCmd,std::string> cmdHelp;
		
		// Constructor
		UserInputHandler(
			std::shared_ptr<DMController> p_dmController,
			std::shared_ptr<ImageStreamManager> p_imageStreamManager);

        // The central loop of the input handler. Handles user input.
        void core();

    private:
		const std::shared_ptr<DMController> mp_DMController;
		const std::shared_ptr<ImageStreamManager> mp_ISManager;
		bool mRunning = true;
		int mPrintareaStart;
		const char * mPrompt = "Enter command: ";
		const char * mAnswerUnknown = "Command not found: ";
        std::string mCurrentCommand;

		// Make default ctor private
		UserInputHandler();
		
		// Flushes the last user input and places the cursor at the cmd input position
        void waitOnNextCmd();
		// Takes the input buffer, parses for known commands and calls the corresponding methods
		void handleInput(char* input);
		
		// ==== CMD methods ====
		// Prints a response to the user, telling him that the requested command does not exist
		void handleUnknownCmd(std::string cmd);
		// Prints a help list
		void execCmdHelp();
		// Activates/deactivates DM input from ISIO
		void execCmdArmUnarm(bool arm);
		// Starts the relax routine of the DM
		void execCmdRelax();
		
		// === Print methods ===
		// Clears the print area
		void clearPrintArea();
		// Clears the line below the user input prompt
		void clearResponseLine();

};

#endif // USERINPUTHANDLER_H
