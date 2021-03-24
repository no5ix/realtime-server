# -*- coding: utf-8 -*-
import bson.objectid as objectid
import uuid


class IdManagerInterface(object):
    @staticmethod
    def genid():
        raise NotImplementedError

    @staticmethod
    def str2id(string):
        raise NotImplementedError

    @staticmethod
    def id2str(uid):
        raise NotImplementedError

    @staticmethod
    def bytes2id(bytes):
        raise NotImplementedError

    @staticmethod
    def id2bytes(uid):
        raise NotImplementedError

    @staticmethod
    def get_id_type():
        raise NotImplementedError

    @staticmethod
    def is_id_type(obj):
        raise NotImplementedError


class IdManagerImpl_UUID(IdManagerInterface):
    """
    IdManager负责给Entity产生一个唯一的id
    """
    @staticmethod
    def genid():
        return uuid.uuid1()

    @staticmethod
    def str2id(string):
        return uuid.UUID(string)

    @staticmethod
    def id2str(uid):
        return str(uid)

    @staticmethod
    def bytes2id(bytes):
        return uuid.UUID(bytes = bytes)

    @staticmethod
    def id2bytes(uid):
        return uid.bytes

    @staticmethod
    def get_id_type():
        return uuid.UUID

    @staticmethod
    def is_id_type(obj):
        return isinstance(obj, uuid.UUID)


class IdManagerImpl_ObjectId(IdManagerInterface):
    """
    IdManager负责给Entity产生一个唯一的id
    """
    @staticmethod
    def genid():
        return objectid.ObjectId()

    @staticmethod
    def str2id(string):
        return objectid.ObjectId(string)

    @staticmethod
    def id2str(uid):
        return str(uid)

    @staticmethod
    def bytes2id(bytes):
        return objectid.ObjectId(bytes)

    @staticmethod
    def id2bytes(uid):
        return uid.binary

    @staticmethod
    def get_id_type():
        return objectid.ObjectId

    @staticmethod
    def is_id_type(obj):
        return isinstance(obj, objectid.ObjectId)


IdManager = IdManagerImpl_ObjectId
