#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cctype>
using namespace std;

struct Position
{
    int row;
    int col;

    Position()
    {
        row = 0;
        col = 0;
    }

    Position(int new_row, int new_col)
    {
        row = new_row;
        col = new_col;
    }

    bool operator==(const Position& other) const
    {
        return row == other.row && col == other.col;
    }

    Position operator+(const Position& other) const
    {
        return Position(row + other.row, col + other.col);
    }
};

string lower_text(string text)
{
    for (int i = 0; i < text.length(); i++)
    {
        text[i] = static_cast<char>(
            tolower(static_cast<unsigned char>(text[i]))
        );
    }

    return text;
}

bool is_number(string text)
{
    if (text.length() == 0)
    {
        return false;
    }

    for (int i = 0; i < text.length(); i++)
    {
        if (!isdigit(static_cast<unsigned char>(text[i])))
        {
            return false;
        }
    }

    return true;
}

int string_to_int(string text)
{
    int value = 0;

    for (int i = 0; i < text.length(); i++)
    {
        value = value * 10 + (text[i] - '0');
    }

    return value;
}

Position direction_step(char direction)
{
    if (direction == '^')
    {
        return Position(-1, 0);
    }

    if (direction == 'v')
    {
        return Position(1, 0);
    }

    if (direction == '<')
    {
        return Position(0, -1);
    }

    return Position(0, 1);
}

char opposite_direction(char direction)
{
    if (direction == '^')
    {
        return 'v';
    }

    if (direction == 'v')
    {
        return '^';
    }

    if (direction == '<')
    {
        return '>';
    }

    return '<';
}

char clockwise_direction(char direction)
{
    if (direction == '^')
    {
        return '>';
    }

    if (direction == '>')
    {
        return 'v';
    }

    if (direction == 'v')
    {
        return '<';
    }

    return '^';
}

class Guard
{
protected:
    Position position;
    char direction;

public:
    Guard(Position start, char start_direction)
    {
        position = start;
        direction = start_direction;
    }

    virtual ~Guard()
    {
    }

    Position get_position() const
    {
        return position;
    }

    void set_position(Position new_position)
    {
        position = new_position;
    }

    char get_direction() const
    {
        return direction;
    }

    virtual void turn_when_blocked() = 0;

    virtual string get_type() const = 0;
};

class NormalGuard : public Guard
{
public:
    NormalGuard(Position start, char start_direction)
        : Guard(start, start_direction)
    {
    }

    void turn_when_blocked() override
    {
        direction = opposite_direction(direction);
    }

    string get_type() const override
    {
        return "normal guard";
    }
};

class PatrolGuard : public Guard
{
public:
    PatrolGuard(Position start, char start_direction)
        : Guard(start, start_direction)
    {
    }

    void turn_when_blocked() override
    {
        direction = clockwise_direction(direction);
    }

    string get_type() const override
    {
        return "patrol guard";
    }
};

class Level
{
private:
    string name;
    vector<string> map;
    Position player;
    Position goal;
    vector<unique_ptr<Guard>> guards;
    vector<bool> door_open;

public:
    Level(string level_name, vector<string> level_map)
    {
        name = level_name;
        map = level_map;
        door_open.resize(10, false);

        read_level();
    }

    void play()
    {
        while (true)
        {
            print_map();

            if (player_seen())
            {
                cout << "A guard saw you. Mission failed." << endl;
                return;
            }

            cout << "Move with W A S D." << endl;
            cout << "Type inspect to inspect a tile." << endl;
            cout << "Type quit to return to the menu." << endl;
            cout << "Choice: ";

            string input;
            getline(cin, input);
            input = lower_text(input);

            if (input == "quit" || input == "q")
            {
                return;
            }

            if (input == "inspect")
            {
                inspect_tile();
                continue;
            }

            Position movement;

            if (input == "w")
            {
                movement = Position(-1, 0);
            }
            else if (input == "s")
            {
                movement = Position(1, 0);
            }
            else if (input == "a")
            {
                movement = Position(0, -1);
            }
            else if (input == "d")
            {
                movement = Position(0, 1);
            }
            else
            {
                cout << "Invalid input." << endl;
                continue;
            }

            Position next_player = player + movement;

            if (blocks_movement(next_player))
            {
                cout << "Your path is blocked." << endl;
                continue;
            }

            if (guard_at(next_player) != -1)
            {
                cout << "You walked into a guard. Mission failed." << endl;
                return;
            }

            player = next_player;
            activate_switch(player);

            if (player == goal)
            {
                print_map();
                cout << "You stole the information. Mission complete." << endl;
                return;
            }

            if (player_seen())
            {
                print_map();
                cout << "A guard saw you. Mission failed." << endl;
                return;
            }

            move_guards();

            if (guard_at(player) != -1)
            {
                print_map();
                cout << "A guard caught you. Mission failed." << endl;
                return;
            }

            if (player_seen())
            {
                print_map();
                cout << "A guard saw you. Mission failed." << endl;
                return;
            }
        }
    }

private:
    void read_level()
    {
        for (int row = 0; row < map.size(); row++)
        {
            for (int col = 0; col < map[row].length(); col++)
            {
                char cell = map[row][col];

                if (cell == '@')
                {
                    player = Position(row, col);
                    map[row][col] = ' ';
                }
                else if (cell == '$')
                {
                    goal = Position(row, col);
                }
                else if (
                    cell == '^' ||
                    cell == 'v' ||
                    cell == '<' ||
                    cell == '>'
                )
                {
                    guards.push_back(
                        make_unique<NormalGuard>(
                            Position(row, col),
                            cell
                        )
                    );

                    map[row][col] = ' ';
                }
                else if (
                    cell == 'U' ||
                    cell == 'D' ||
                    cell == 'L' ||
                    cell == 'R'
                )
                {
                    char direction = '>';

                    if (cell == 'U')
                    {
                        direction = '^';
                    }
                    else if (cell == 'D')
                    {
                        direction = 'v';
                    }
                    else if (cell == 'L')
                    {
                        direction = '<';
                    }

                    guards.push_back(
                        make_unique<PatrolGuard>(
                            Position(row, col),
                            direction
                        )
                    );

                    map[row][col] = ' ';
                }
            }
        }
    }

    void print_map() const
    {
        vector<string> display = map;

        for (int row = 0; row < display.size(); row++)
        {
            for (int col = 0; col < display[row].length(); col++)
            {
                char cell = display[row][col];

                if (is_door(cell))
                {
                    int group = cell - '0';

                    if (door_open[group])
                    {
                        display[row][col] = ' ';
                    }
                }
            }
        }

        display[goal.row][goal.col] = '$';

        for (int i = 0; i < guards.size(); i++)
        {
            Position guard_position = guards[i]->get_position();

            display[guard_position.row][guard_position.col] =
                guards[i]->get_direction();
        }

        display[player.row][player.col] = '@';

        cout << endl;
        cout << name << endl;

        for (int row = 0; row < display.size(); row++)
        {
            cout << display[row] << endl;
        }

        cout << endl;
    }

    bool outside_map(Position position) const
    {
        if (position.row < 0 || position.row >= map.size())
        {
            return true;
        }

        if (
            position.col < 0 ||
            position.col >= map[position.row].length()
        )
        {
            return true;
        }

        return false;
    }

    bool is_door(char cell) const
    {
        return cell >= '1' && cell <= '9';
    }

    bool is_switch(char cell) const
    {
        return cell >= 'a' && cell <= 'i';
    }

    bool door_is_closed(Position position) const
    {
        if (outside_map(position))
        {
            return true;
        }

        char cell = map[position.row][position.col];

        if (!is_door(cell))
        {
            return false;
        }

        int group = cell - '0';

        return !door_open[group];
    }

    bool blocks_movement(Position position) const
    {
        if (outside_map(position))
        {
            return true;
        }

        char cell = map[position.row][position.col];

        if (cell == '#')
        {
            return true;
        }

        if (door_is_closed(position))
        {
            return true;
        }

        return false;
    }

    bool blocks_guard(Position position) const
    {
        if (blocks_movement(position))
        {
            return true;
        }

        if (position == goal)
        {
            return true;
        }

        if (guard_at(position) != -1)
        {
            return true;
        }

        return false;
    }

    bool blocks_vision(Position position) const
    {
        if (outside_map(position))
        {
            return true;
        }

        char cell = map[position.row][position.col];

        if (cell == '#')
        {
            return true;
        }

        if (door_is_closed(position))
        {
            return true;
        }

        if (position == goal)
        {
            return true;
        }

        if (guard_at(position) != -1)
        {
            return true;
        }

        return false;
    }

    int guard_at(Position position) const
    {
        for (int i = 0; i < guards.size(); i++)
        {
            if (guards[i]->get_position() == position)
            {
                return i;
            }
        }

        return -1;
    }

    void activate_switch(Position position)
    {
        if (outside_map(position))
        {
            return;
        }

        char cell = map[position.row][position.col];

        if (!is_switch(cell))
        {
            return;
        }

        int group = cell - 'a' + 1;
        door_open[group] = !door_open[group];

        if (door_open[group])
        {
            cout << "Door group " << group << " opened." << endl;
        }
        else
        {
            cout << "Door group " << group << " closed." << endl;
        }
    }

    bool player_seen() const
    {
        for (int i = 0; i < guards.size(); i++)
        {
            Position step =
                direction_step(guards[i]->get_direction());

            Position check =
                guards[i]->get_position() + step;

            while (!outside_map(check))
            {
                if (check == player)
                {
                    return true;
                }

                if (blocks_vision(check))
                {
                    break;
                }

                check = check + step;
            }
        }

        return false;
    }

    void move_guards()
    {
        for (int i = 0; i < guards.size(); i++)
        {
            Position step =
                direction_step(guards[i]->get_direction());

            Position next =
                guards[i]->get_position() + step;

            if (next == player)
            {
                guards[i]->set_position(next);
                continue;
            }

            if (blocks_guard(next))
            {
                guards[i]->turn_when_blocked();

                step = direction_step(
                    guards[i]->get_direction()
                );

                next =
                    guards[i]->get_position() + step;
            }

            if (next == player)
            {
                guards[i]->set_position(next);
                continue;
            }

            if (!blocks_guard(next))
            {
                guards[i]->set_position(next);
                activate_switch(next);
            }
        }
    }

    bool read_number_input(string prompt, int& value) const
    {
        string text;

        cout << prompt;
        getline(cin, text);

        if (!is_number(text))
        {
            cout << "Invalid number." << endl;
            return false;
        }

        value = string_to_int(text);
        return true;
    }

    void inspect_tile() const
    {
        int row;
        int col;

        if (!read_number_input("Enter row: ", row))
        {
            return;
        }

        if (!read_number_input("Enter column: ", col))
        {
            return;
        }

        Position inspected(row, col);

        if (outside_map(inspected))
        {
            cout << "That position is outside the level." << endl;
            return;
        }

        if (inspected == player)
        {
            cout << "This tile contains the player." << endl;
            return;
        }

        int guard_index = guard_at(inspected);

        if (guard_index != -1)
        {
            cout << "This tile contains a "
                 << guards[guard_index]->get_type()
                 << "." << endl;

            cout << "The guard is facing "
                 << guards[guard_index]->get_direction()
                 << "." << endl;

            if (
                guards[guard_index]->get_type() ==
                "patrol guard"
            )
            {
                cout << "This guard turns clockwise when blocked."
                     << endl;
            }
            else
            {
                cout << "This guard flips direction when blocked."
                     << endl;
            }

            return;
        }

        char cell = map[row][col];

        if (cell == '#')
        {
            cout << "This tile contains a wall." << endl;
        }
        else if (inspected == goal)
        {
            cout << "This tile contains the goal." << endl;
        }
        else if (is_door(cell))
        {
            int group = cell - '0';

            cout << "This tile contains door group "
                 << group << "." << endl;

            if (door_open[group])
            {
                cout << "The door is open." << endl;
            }
            else
            {
                cout << "The door is closed." << endl;
            }
        }
        else if (is_switch(cell))
        {
            int group = cell - 'a' + 1;

            cout << "This tile contains a switch for door group "
                 << group << "." << endl;
        }
        else
        {
            cout << "This tile is empty." << endl;
        }
    }
};

int main()
{
    bool running = true;

    while (running)
    {
        cout << endl;
        cout << "Ultra-Spy Portfolio 2" << endl;
        cout << "1) Multiple Door Groups" << endl;
        cout << "2) Rectangle Patrol" << endl;
        cout << "3) Complex Patrol" << endl;
        cout << "4) Guard Switch Test" << endl;
        cout << "5) Quit" << endl;
        cout << "Choice: ";

        string choice;
        getline(cin, choice);
        choice = lower_text(choice);

        vector<string> level_one =
        {
            "############",
            "#@ a 1   $ #",
            "# #######  #",
            "# b 2      #",
            "#          #",
            "############"
        };

        vector<string> level_two =
        {
            "############",
            "#@        $#",
            "#  ######  #",
            "#  #    #  #",
            "#  # R  #  #",
            "#  #    #  #",
            "#  ######  #",
            "############"
        };

        vector<string> level_three =
        {
            "############",
            "#@      $  #",
            "#  ###     #",
            "#  # R ### #",
            "#    #     #",
            "# ##   ##  #",
            "#          #",
            "############"
        };

        vector<string> level_four =
        {
            "############",
            "#@       $ #",
            "# #######  #",
            "# > a 1    #",
            "#          #",
            "############"
        };

        if (
            choice == "1" ||
            choice == "multiple door groups"
        )
        {
            Level level(
                "Level 1: Multiple Door Groups",
                level_one
            );

            level.play();
        }
        else if (
            choice == "2" ||
            choice == "rectangle patrol"
        )
        {
            Level level(
                "Level 2: Rectangle Patrol",
                level_two
            );

            level.play();
        }
        else if (
            choice == "3" ||
            choice == "complex patrol"
        )
        {
            Level level(
                "Level 3: Complex Patrol",
                level_three
            );

            level.play();
        }
        else if (
            choice == "4" ||
            choice == "guard switch test"
        )
        {
            Level level(
                "Level 4: Guard Switch Test",
                level_four
            );

            level.play();
        }
        else if (
            choice == "5" ||
            choice == "quit"
        )
        {
            running = false;
        }
        else
        {
            cout << "Invalid choice." << endl;
        }
    }

    cout << "Thanks for playing." << endl;

    return 0;
}
