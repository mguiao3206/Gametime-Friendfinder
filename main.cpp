#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <queue>
#include <iomanip>
#include <limits>

using namespace std;


struct Game {
    string name;
    int playtime;

    Game(string n, int p) : name(n), playtime(p) {}
};

struct UserProfile {
    string username;
    vector<Game> games;
    map<int, int> hourlyPlaytime;
    int totalPlaytime;

    UserProfile(string name) : username(name), totalPlaytime(0) {}

    void addGame(const string& name, int playtime) {
        games.emplace_back(name, playtime);
        totalPlaytime += playtime;
    }

    void addHourlyPlaytime(int hour, int minutes) {
        hourlyPlaytime[hour] += minutes;
    }
};

double calculateSimilarity(const UserProfile& user1, const UserProfile& user2) {
    unordered_set<string> games1, games2;
    for (const auto& game : user1.games) games1.insert(game.name);
    for (const auto& game : user2.games) games2.insert(game.name);

    unordered_set<string> intersection;
    for (const auto& game : games1) {
        if (games2.count(game)) {
            intersection.insert(game);
        }
    }

    double gameSimilarity = intersection.empty() ? 0 :
        static_cast<double>(intersection.size()) /
        (games1.size() + games2.size() - intersection.size());

    double playtimeSimilarity = 1.0 -
        (static_cast<double>(abs(user1.totalPlaytime - user2.totalPlaytime)) /
        max(user1.totalPlaytime, user2.totalPlaytime));

    double dotProduct = 0.0;
    double norm1 = 0.0, norm2 = 0.0;

    for (int hour = 0; hour < 24; hour++) {
        int p1 = user1.hourlyPlaytime.count(hour) ? user1.hourlyPlaytime.at(hour) : 0;
        int p2 = user2.hourlyPlaytime.count(hour) ? user2.hourlyPlaytime.at(hour) : 0;

        dotProduct += p1 * p2;
        norm1 += p1 * p1;
        norm2 += p2 * p2;
    }

    double hourlySimilarity = (norm1 == 0 || norm2 == 0) ? 0 :
        dotProduct / (sqrt(norm1) * sqrt(norm2));

    const double gameWeight = 0.4;
    const double playtimeWeight = 0.3;
    const double hourlyWeight = 0.3;

    return gameWeight * gameSimilarity +
           playtimeWeight * playtimeSimilarity +
           hourlyWeight * hourlySimilarity;
}

vector<pair<string, double>> findSimilarUsers(
    const UserProfile& targetUser,
    const vector<UserProfile>& allUsers,
    int maxSuggestions) {

    using UserSimilarity = pair<double, string>;
    priority_queue<
        UserSimilarity,
        vector<UserSimilarity>,
        greater<UserSimilarity>
    > topUsers;

    for (const auto& user : allUsers) {
        if (user.username != targetUser.username) {
            double similarity = calculateSimilarity(targetUser, user);

            if (topUsers.size() < maxSuggestions) {
                topUsers.emplace(similarity, user.username);
            } else if (similarity > topUsers.top().first) {
                topUsers.pop();
                topUsers.emplace(similarity, user.username);
            }
        }
    }

    vector<pair<string, double>> result(topUsers.size());
    for (int i = topUsers.size() - 1; i >= 0; i--) {
        result[i] = {topUsers.top().second, topUsers.top().first};
        topUsers.pop();
    }

    return result;
}

void printUserProfile(const UserProfile& user) {
    cout << "\nUser Profile: " << user.username << endl;
    cout << "Total Playtime: " << user.totalPlaytime/60 << " hours" << endl;

    cout << "Games (" << user.games.size() << "):" << endl;
    for (const auto& game : user.games) {
        cout << "  - " << left << setw(20) << game.name
             << " (" << game.playtime/60 << " hrs)" << endl;
    }

    cout << "Peak Play Hours: ";
    for (const auto& [hour, minutes] : user.hourlyPlaytime) {
        if (minutes > 0) {
            cout << hour << ":00 (" << minutes << " mins), ";
        }
    }
    cout << endl;
}

UserProfile createUserProfile() {
    string username;
    cout << "Enter username: ";
    getline(cin, username);
    UserProfile user(username);

    cout << "\nAdding games (enter 'done' when finished):" << endl;
    while (true) {
        string gameName;
        cout << "Game name: ";
        getline(cin, gameName);
        if (gameName == "done") break;

        int playtime;
        cout << "Playtime in hours: ";
        while (!(cin >> playtime)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number: ";
        }
        cin.ignore();

        user.addGame(gameName, playtime * 60);
    }

    cout << "\nEnter playtime by hour (0-23, enter -1 when finished):" << endl;
    while (true) {
        int hour, minutes;
        cout << "Hour (0-23): ";
        while (!(cin >> hour) || hour < -1 || hour > 23) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid hour. Enter 0-23 or -1 to finish: ";
        }
        if (hour == -1) break;

        cout << "Minutes played at " << hour << ":00: ";
        while (!(cin >> minutes) || minutes < 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid minutes. Enter positive number: ";
        }
        cin.ignore();

        user.addHourlyPlaytime(hour, minutes);
    }

    return user;
}

void mainMenu() {
    vector<UserProfile> users;
    UserProfile* targetUser = nullptr;

    while (true) {
        cout << "\n=== Friend-Finder Menu ===" << endl;
        cout << "1. Create target user profile" << endl;
        cout << "2. Add comparison user profile" << endl;
        cout << "3. View all profiles" << endl;
        cout << "4. Find similar users" << endl;
        cout << "5. Exit" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                cout << "\nCreating target user profile:" << endl;
                users.push_back(createUserProfile());
                targetUser = &users.back();
                cout << "\nTarget user created successfully!" << endl;
                break;
            }
            case 2: {
                cout << "\nAdding comparison user profile:" << endl;
                users.push_back(createUserProfile());
                cout << "\nComparison user added successfully!" << endl;
                break;
            }
            case 3: {
                if (users.empty()) {
                    cout << "No profiles to display." << endl;
                } else {
                    cout << "\n=== All User Profiles ===" << endl;
                    for (const auto& user : users) {
                        printUserProfile(user);
                    }
                }
                break;
            }
            case 4: {
                if (!targetUser || users.size() < 2) {
                    cout << "Need at least a target user and one comparison user." << endl;
                } else {
                    int maxSuggestions;
                    cout << "How many suggestions would you like? ";
                    cin >> maxSuggestions;
                    cin.ignore();

                    auto similarUsers = findSimilarUsers(*targetUser, users, maxSuggestions);

                    cout << "\n=== TOP " << similarUsers.size() << " SIMILAR USERS ===" << endl;
                    for (const auto& [username, similarity] : similarUsers) {
                        cout << "\nUser: " << username << endl;
                        cout << "Similarity Score: " << fixed << setprecision(2) << similarity*100 << "%" << endl;

                        for (const auto& user : users) {
                            if (user.username == username) {
                                printUserProfile(user);
                                break;
                            }
                        }
                    }
                }
                break;
            }
            case 5: {
                cout << "Exiting program." << endl;
                return;
            }
            default: {
                cout << "Invalid choice. Please try again." << endl;
                break;
            }
        }
    }
}

int main() {
    mainMenu();
    return 0;
}