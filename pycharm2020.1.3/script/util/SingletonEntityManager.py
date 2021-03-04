

class SingletonEntityManager(object):

    _instance = None

    def __int__(self):
        pass

    @classmethod
    def instance(cls):
        if cls._instance is None:
            cls._instance = SingletonEntityManager()
            return cls._instance

    def register_centers_and_stubs(self, game_server_name, cb):
        pass
