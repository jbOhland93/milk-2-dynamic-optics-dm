#include "../headers/UserInputHandler.h"

#include <ncurses.h>
#include <sstream>
#include <iterator>

using namespace std;

vector<userCmd> UserInputHandler::cmdList = {
	userCmd::CMD_HELP,
    userCmd::CMD_ARM,
	userCmd::CMD_DISARM,
    userCmd::CMD_RELAX,
    userCmd::CMD_STRESSTEST,
    userCmd::CMD_SET_FPSCAP,
    userCmd::CMD_SET_DC_OFFSET,
	userCmd::CMD_QUIT,
};

map<string,userCmd> UserInputHandler::cmdStrings = {
	{"help", userCmd::CMD_HELP},
    {"arm", userCmd::CMD_ARM},
    {"disarm", userCmd::CMD_DISARM},
    {"relax", userCmd::CMD_RELAX},
    {"stress", userCmd::CMD_STRESSTEST},
    {"setFPS", userCmd::CMD_SET_FPSCAP},
    {"setDC", userCmd::CMD_SET_DC_OFFSET},
	{"quit", userCmd::CMD_QUIT},
};

map<userCmd,string> UserInputHandler::cmdHelp = {
	{userCmd::CMD_HELP, "Shows a list of available commands"},
    {userCmd::CMD_ARM, "Enables DM control via ISIO stream (default)"},
    {userCmd::CMD_DISARM, "Disables DM control via ISIO stream"},
    {userCmd::CMD_RELAX, "Starts the DM relaxation routine"},
    {userCmd::CMD_STRESSTEST, "Performs a simple stresstest and prints telemetry"},
    {userCmd::CMD_SET_FPSCAP, "Limits the DM FPS; values <= 0 equal freerunning mode"},
    {userCmd::CMD_SET_DC_OFFSET, "Applies a DC offset over all actuators"},
	{userCmd::CMD_QUIT, "Quits the programm"},
};

UserInputHandler::UserInputHandler(
    std::shared_ptr<DMController> p_dmController,
    std::shared_ptr<ImageStreamManager> p_imageStreamManager)
    :   mp_DMController(p_dmController),
        mp_ISManager(p_imageStreamManager)
{
}

void UserInputHandler::core()
{
    printf("== Initialization done! ==\n");
    initscr();
    wmove(stdscr, 0, 0);
    addstr("=== Welcome to the CLI of the milk-2-dynamic-optics-dm application. ===\n");
    addstr("\nEnter any string and hit enter to display the response.\n");
    addstr("Enter \"quit\" to exit the CLI.\n");
    addstr("Enter \"help\" for a list of comand options.\n\n");
    m_printareaStart = getcury(stdscr);

    mvprintw(LINES-2, 0, m_prompt);

    m_armed = true;
    m_dmPollThread = std::thread(&UserInputHandler::ISIOtoDM, this);
    while (m_running) {
        waitOnNextCmd();

        char str[1024];
        getstr(str);
		handleInput(str);
    }
    endwin();

    printf("== Quit programm. Start cleaning up ... ==\n");

    // Stop DM polling, if active
    if (m_armed)
    {
        m_armed = false;
        m_dmPollThread.join();
    }
}

void UserInputHandler::waitOnNextCmd()
{
	wmove(stdscr, LINES-2, 0);
	clrtoeol();
    addstr(m_prompt);
}

void UserInputHandler::handleInput(char* input)
{
    istringstream iss(input);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};
    if (tokens.size() == 0)
        return; // Empty command string - perhaps user pressed enter.
    string cmd = tokens.at(0);

	if (cmdStrings.find(cmd) == cmdStrings.end()) {
		handleUnknownCmd(tokens.at(0));
	} else {
		switch(cmdStrings[cmd]) {
		case userCmd::CMD_HELP:
			execCmdHelp();
			break;
        case userCmd::CMD_ARM:
			execCmdArmDisarm(true);
			break;
        case userCmd::CMD_DISARM:
			execCmdArmDisarm(false);
			break;
        case userCmd::CMD_RELAX:
            execCmdRelax();
            break;
        case userCmd::CMD_STRESSTEST:
            execCmdStresstest(tokens);
            break;
        case userCmd::CMD_SET_FPSCAP:
            execCmdSetFPScap(tokens);
            break;
        case userCmd::CMD_SET_DC_OFFSET:
            execCmdDCoffset(tokens);
            break;
		case userCmd::CMD_QUIT:
			m_running = false;
			break;
		default:
			mvprintw(LINES-1, 0, "Command found but no switch case implemented: ");
			clrtoeol();
			addstr(cmd.c_str());
		}
    }
}

void UserInputHandler::handleUnknownCmd(string cmd)
{
    clearResponseLine();
	addstr(m_answerUnknown);
	addstr(cmd.c_str());
}

void UserInputHandler::execCmdHelp()
{
    int descrCol = 32;

    clearPrintArea();
    addstr("The following commands are available:\n");

	map<string, userCmd>::iterator it;
	for (it = cmdStrings.begin(); it != cmdStrings.end(); it++)
	{
        addstr(it->first.c_str());
        wmove(stdscr, getcury(stdscr), descrCol);
        addstr(cmdHelp[it->second].c_str());
        addstr("\n");
	}
    clearResponseLine();
}

void UserInputHandler::execCmdArmDisarm(bool start)
{
    clearResponseLine();
    if (start)
    {
        if (m_armed)
            mvprintw(LINES-1, 0, "DM is already armed, nothing to be done.");
        else
        {
            int w,h;
            getmaxyx(stdscr, h, w);
            
            WINDOW* win = newwin(0,0,w,h);
            overwrite(stdscr, win);

            m_armed = true;
            m_dmPollThread = std::thread(&UserInputHandler::ISIOtoDM, this);

            overwrite(win, stdscr);
            addstr("DM armed for ISIO input.");
        }
    }
    else
        if (m_armed)
        {
            m_armed = false;
            m_dmPollThread.join();
            mvprintw(LINES-1, 0, "DM disarmed and idle.");
        }
        else
        {
            addstr("DM is already disarmed, nothing to be done.");
        }
}

void UserInputHandler::execCmdRelax()
{
    clearResponseLine();
    if (m_armed)
        mvprintw(LINES-1, 0, "DM is armed for ISIO control. Disarm before relaxing.");
    else
    {
        // Print a WIP notification
        wmove(stdscr, LINES-1, 0);
        clrtoeol();
        mvprintw(LINES-1, 0, "Relaxation routine running...");
        wrefresh(stdscr);
        // Launch relaxing routine
        mp_DMController->relaxDM();
        // Print a completion notification
        wmove(stdscr, LINES-1, 0);
        clrtoeol();
        mvprintw(LINES-1, 0, "Relaxation routine finished!");
    }
}

void UserInputHandler::execCmdStresstest(std::vector<string> args)
{
    clearResponseLine();
    if (m_armed)
        mvprintw(LINES-1, 0, "DM is armed for ISIO control. Disarm before performing stresstest.");
    else
    {
        if (args.size() < 2)
            mvprintw(LINES-1, 0, "Please add the desired number of pokes.");
        else
        {
            // Print a WIP notification
            int numPokes = stoi(args.at(1));
            if (numPokes < 1)
                mvprintw(LINES-1, 0, "Number of pokes must be > 0.");
            else
            {
                wmove(stdscr, LINES-1, 0);
                clrtoeol();
                mvprintw(LINES-1, 0, "Stresstest running...");
                wrefresh(stdscr);

                // Launch relaxing routine
                int64_t duration_us = mp_DMController->stressTest(numPokes);
                // Print a completion notification
                std::stringstream ss;
                ss  << "Stresstest finished: " << numPokes
                    << " pokes in " << duration_us
                    << "us (" << (float) duration_us/(numPokes - 1
                    )
                    << "us per poke).";
                wmove(stdscr, LINES-1, 0);
                clrtoeol();
                mvprintw(LINES-1, 0, ss.str().c_str());
            }
        }
    }
}

void UserInputHandler::execCmdSetFPScap(std::vector<std::string> args)
{
    clearResponseLine();
    if (args.size() < 2)
        mvprintw(LINES-1, 0, "Please add the desired FPS (float) to the cmd.");
    else
    {
        float fps = stof(args.at(1));
        mp_DMController->setFrameRateCap(fps);
        // Print a completion notification
        std::stringstream ss;
        if (fps <= 0)
            ss << "DM set to freerunning mode.";
        else
            ss  << "Framerate set to " << fps << " Hz.";
        wmove(stdscr, LINES-1, 0);
        clrtoeol();
        mvprintw(LINES-1, 0, ss.str().c_str());
    }
}

void UserInputHandler::execCmdDCoffset(std::vector<std::string> args)
{
    clearResponseLine();
    if (args.size() < 2)
        mvprintw(LINES-1, 0, "Please add the desired DC offset (float) to the cmd.");
    else
    {
        float dcOff = stof(args.at(1));
        mp_DMController->setCDoffset(dcOff);
        // Print a completion notification
        std::stringstream ss;
        ss  << "DC offset of " << dcOff << " applied.";
        wmove(stdscr, LINES-1, 0);
        clrtoeol();
        mvprintw(LINES-1, 0, ss.str().c_str());
    }
}

void UserInputHandler::clearPrintArea()
{
    wmove(stdscr, m_printareaStart, 0);
    int curY = m_printareaStart;
    while (curY < LINES-2) {
        clrtoeol();
        curY++;
        wmove(stdscr, curY, 0);
    }
    wmove(stdscr, m_printareaStart, 0);
}

void UserInputHandler::clearResponseLine()
{
    wmove(stdscr, LINES-1, 0);
    clrtoeol();
}

void UserInputHandler::ISIOtoDM()
{
    while(m_armed)
    {
        // Update DM if a new image is available in the next 100 ms.
        if (mp_ISManager->waitForNextImage(100000))
            mp_DMController->setActuatorValues(mp_ISManager->getData());
    }
}