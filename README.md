# 🟩 Chrome Password Retrieval 🟩

**Chrome Password Retrieval is for retrieving decrypted chrome user data.**

## Usage

```c++
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
```

## Example.png

![alt text](https://github.com/yurtrimu/chrome-password/blob/main/Capture.PNG?raw=true)

## Operating System
- **<Windows 7 - 🟦 Untested**
- **Windows 7 - 🟦 Untested**
- **Windows 10 - 🟩 Working**
- **Windows 11 - 🟦 Untested**

## Requirements

**You need to have the OpenSSL library installed and linked to your compiler.**

- You could download the OpenSSL library from [OpenSSL website](https://www.openssl.org/source/).

## Linker

- **libssl.lib**
- **libcrypto.lib**
- **crypt32.lib**

## Contributing

**Pull requests are welcome.**

## Legal Disclaimer
🟥 **The content provided is for educational and informational purposes only.** 🟥
