import os
from service import Service


class UserInterface:
    def __init__(self):
        self.service = Service()

    def _register(self):
        name = input('Name: ')
        password = input('Password: ')
        self.service.register(name, password)

    def _login(self):
        name = input('Name: ')
        password = input('Password: ')
        return self.service.login(name, password)

    def logout(self, cookie):
        self.service.logout(cookie)

    def login_menu(self):
        while True:
            os.system('clear')
            print('===== Account menu =====')
            print('1) Registration')
            print('2) Log in')
            print('3) Exit')

            choise = input()

            if choise == '1':
                self._register()
            if choise == '2':
                cookie = self._login()
                if cookie:
                    return cookie
            if choise == '3':
                exit(0)

    def show_contacts(self, cookie):
        os.system('clear')
        print('Contacts:')
        contacts = self.service.get_contacts(cookie)
        if contacts:
            for elem in contacts:
                print('{}\t{}\t{}\t{}'.format(elem['id'], elem['name'], elem['email'], elem['phone_number']))
        input()

    def add_contact(self, cookie):
        os.system('clear')
        print('Add contact:')
        name = input('Name: ')
        email = input('Email: ')
        phone_number = input('Phone number: ')
        self.service.add_contact(cookie, name, email, phone_number)

    def update_contact(self, cookie):
        os.system('clear')
        print('Update contacts:')
        contacts = self.service.get_contacts(cookie)
        if contacts:
            for elem in contacts:
                print('{}\t{}\t{}\t{}'.format(elem['id'], elem['name'], elem['email'], elem['phone_number']))

            while True:
                id = input('Id (e - exit): ')
                if id != 'e':
                    needed = list(filter(lambda i: i['id'] == id, contacts))
                    if not needed:
                        print('Id not found')
                    else:
                        needed = needed[0]

                        new_name = input('New name (empty - does not update): ')
                        new_email = input('New email (empty - does not update): ')
                        new_phone_number = input('New phone number (empty - does not update): ')

                        self.service.update_contact(
                            cookie,
                            id,
                            new_name if new_name else needed['name'],
                            new_email if new_email else needed['email'],
                            new_phone_number if new_phone_number else needed['phone_number'],
                        )
                else:
                    return

    def delete_contact(self, cookie):
        os.system('clear')
        print('Delete contacts:')
        contacts = self.service.get_contacts(cookie)
        if contacts:
            for elem in contacts:
                print('{}\t{}\t{}\t{}'.format(elem['id'], elem['name'], elem['email'], elem['phone_number']))

            while True:
                id = input('Id (e - exit): ')

                if id != 'e':
                    needed = list(filter(lambda i: i['id'] == id, contacts))
                    if not needed:
                        print('Id not found')
                    else:
                        self.service.delete_contact(cookie, id)
                else:
                    return

    def main_menu(self):
        cookie = self.login_menu()

        while True:
            os.system('clear')
            print('===== Main menu =====')
            print('1) Show contacts')
            print('2) Add contact')
            print('3) Update contact')
            print('4) Delete contact')
            print('5) Exit')

            choise = input()

            if choise == '1':
                self.show_contacts(cookie)
            if choise == '2':
                self.add_contact(cookie)
            if choise == '3':
                self.update_contact(cookie)
            if choise == '4':
                self.delete_contact(cookie)
            if choise == '5':
                self.logout(cookie)
                return


if __name__ == '__main__':
    ui = UserInterface()
    ui.main_menu()

