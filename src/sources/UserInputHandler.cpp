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
	userCmd::CMD_QUIT,
};

map<string,userCmd> UserInputHandler::cmdStrings = {
	{"help", userCmd::CMD_HELP},
    {"arm", userCmd::CMD_ARM},
    {"disarm", userCmd::CMD_DISARM},
    {"relax", userCmd::CMD_RELAX},
	{"quit", userCmd::CMD_QUIT},
};

map<userCmd,string> UserInputHandler::cmdHelp = {
	{userCmd::CMD_HELP, "Shows a list of available commands"},
    {userCmd::CMD_ARM, "Enables DM control via ISIO stream (default)"},
    {userCmd::CMD_DISARM, "Disables DM control via ISIO stream"},
    {userCmd::CMD_RELAX, "Starts the DM relaxation routine"},
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