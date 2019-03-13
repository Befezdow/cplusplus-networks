import requests


class Service:
    def __init__(self, url='http://127.0.0.1:6080'):
        self.server_url = url

    def register(self, login, password):
        request = {
            'login': login,
            'password': password
        }
        res = requests.post('{}/auth'.format(self.server_url), json=request)
        parsed_json = res.json()
        return int(parsed_json['ok'])

    def login(self, login, password):
        request = {
            'login': login,
            'password': password
        }
        res = requests.get('{}/auth'.format(self.server_url), params=request)
        parsed_json = res.json()
        if int(parsed_json['ok']):
            return parsed_json['id']
        else:
            return None

    def logout(self, cookie):
        request = {
            'id': cookie
        }
        res = requests.delete('{}/auth'.format(self.server_url), params=request)
        parsed_json = res.json()
        return int(parsed_json['ok'])

    def get_contacts(self, cookie):
        request = {
            'id': cookie
        }
        res = requests.get('{}/contacts'.format(self.server_url), params=request)
        parsed_json = res.json()
        if int(parsed_json['ok']):
            return parsed_json['contacts']
        else:
            return None

    def update_contact(self, cookie, record_id, name, email, phone_number):
        request = {
            'id': cookie
        }
        contact_info = {
            'record_id': record_id,
            'name': name,
            'email': email,
            'phone_number': phone_number
        }
        res = requests.put('{}/contacts'.format(self.server_url), params=request, json=contact_info)
        parsed_json = res.json()
        return int(parsed_json['ok'])

    def delete_contact(self, cookie, record_id):
        request = {
            'id': cookie
        }
        contact_info = {
            'record_id': record_id,
        }
        res = requests.delete('{}/contacts'.format(self.server_url), params=request, json=contact_info)
        parsed_json = res.json()
        return int(parsed_json['ok'])

    def add_contact(self, cookie, name, email, phone_number):
        request = {
            'id': cookie
        }
        contact_info = {
            'name': name,
            'email': email,
            'phone_number': phone_number
        }
        res = requests.post('{}/contacts'.format(self.server_url), params=request, json=contact_info)
        parsed_json = res.json()
        return int(parsed_json['ok'])
