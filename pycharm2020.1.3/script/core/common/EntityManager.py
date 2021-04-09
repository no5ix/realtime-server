from __future__ import annotations

import typing

from ..util.UtilApi import Singleton

if typing.TYPE_CHECKING:
	from server_entity.ServerEntity import ServerEntity
from .IdManager import IdManager
from ..mobilelog.LogManager import LogManager
# from IdManager import IdManager


class EntityIdOrLocalId(object):
	entityid_localids = {}
	localid_entityids = {}
	localid_sync = set()

	# server
	@staticmethod
	def clear():
		EntityIdOrLocalId.entityid_localids = {}
		EntityIdOrLocalId.localid_entityids = {}
		EntityIdOrLocalId.localid_sync.clear()

	@staticmethod
	def clear_localid_sync(entityid):
		"""
		清理localid已经同步的标志位，重连接相关的功能需要
		"""
		EntityIdOrLocalId.localid_sync.discard(entityid)

	@staticmethod
	def set_entityid_localid(entityid, localid):
		if localid > 0:
			EntityIdOrLocalId.entityid_localids[entityid] = localid
			EntityIdOrLocalId.localid_entityids[localid] = entityid

	@staticmethod
	def raw_encode(entityid):
		localid = EntityIdOrLocalId.entityid_localids.get(entityid, -1)
		if localid == -1:
			return entityid, localid

		reteid = '' if entityid in EntityIdOrLocalId.localid_sync else entityid
		return reteid, localid

	@staticmethod
	def raw_decode(entityid, localid):
		if localid == -1:
			return entityid, localid
		if entityid is None or entityid == '':
			entityid = EntityIdOrLocalId.localid_entityids.get(localid, '')
			EntityIdOrLocalId.localid_sync.add(entityid)
			return entityid, localid
		entityid = EntityIdOrLocalId.localid_entityids.get(localid, '')
		return entityid, localid

	@staticmethod
	def destroy(entityid):
		try:
			entityid = IdManager.id2bytes(entityid)
		except:
			return
		localid = EntityIdOrLocalId.entityid_localids.get(entityid, -1)
		if localid > 0:
			try:
				EntityIdOrLocalId.localid_sync.discard(entityid)
				del EntityIdOrLocalId.entityid_localids[entityid]
				del EntityIdOrLocalId.localid_entityids[localid]
			except KeyError:
				pass

	# client
	@staticmethod
	def decode(entityid, localid):
		if localid == -1:
			return entityid, localid
		if entityid is not None and entityid != '':
			EntityIdOrLocalId.entityid_localids[entityid] = localid
			EntityIdOrLocalId.localid_entityids[localid] = entityid
		return EntityIdOrLocalId.localid_entityids.get(localid, entityid), localid

	@staticmethod
	def encode(entityid):
		localid = EntityIdOrLocalId.entityid_localids.get(entityid, -1)
		if localid == -1:
			return entityid, -1
		return '', localid


@Singleton
class EntityManager(object):

	def __init__(self):
		# logger for EntityFactory
		self._logger = LogManager.get_logger("server.EntityManager")
		# registered classed for EntityFactory
		# self.entity_classes = {}
		self._entities = {}  # type: typing.Dict[int, ServerEntity]

# class EntityManager(object):
	""" 管理所有的Entity的管理器"""
	# logger = LogManager.get_logger("server.EntityManager") if logger
	# _entities = {}

	# @classmethod
	# def size(cls):
	# 	return len(cls._entities)

	def entitynumber(self, ):
		"""是否存在entity"""
		return len(self._entities)

	def hasentity(self, entityid):
		"""是否存在entity"""
		return entityid in self._entities

	def getentity(self, entityid):
		"""得到entity"""
		return self._entities.get(entityid, None)

	def delentity(self, entityid):
		"""删除entity"""
		try:
			EntityIdOrLocalId.destroy(entityid)
			del self._entities[entityid]
		except KeyError:
			self._logger.warn(" entity id  %s didn't exist", entityid)
		self._logger.info("delentity  entity id  %s ", entityid)

	def addentity(self, entityid, entity, override=False):
		"""增加entity， override为True的时候，会覆盖原有相同id的Entity"""
		# if self._entities.has_key(entityid):
		if entityid in self._entities:
			self._logger.warn(" entity  %s already exist", entityid)
			if not override:
				return
		self._logger.debug("addentity  entity id  %s ", entityid)
		self._entities[entityid] = entity

	def iter_entity(self):
		for entity_id, ent in self._entities.items():
			yield entity_id, ent
		# return cls._entities.itervalues()

	# @classmethod
	# def values(cls):
	# 	return cls._entities.values()
