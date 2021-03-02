from common import gr


class BattleEntity(object):

    def __init__(self):
        self.be_id = 0
        self.bind_entity = None

    def set_puppet_bind_entity(self, pbe):
        self.bind_entity = pbe
        pbe.set_bind_ok()

    def call_server_method(self, method_name, parameters):
        # if not gr.bind_entity:
        #     return
        # if gr.IS_RECORD_PLAY:
        #     return
        # if not self.dungeon:
        #     raise StandardError()
        # if const.PRINT_PROTO:
        #     if method_name not in ('move_to', 'CompMovement.get_rtt_info', 'CompLocalEntityRouter.route', 'ride_move_to'):
        #         if gr.is_client:
        #             gr.logger.warn("[aoi c=>s]: %s", method_name)

        # gr.bind_entity.call_server_method(
        self.bind_entity.call_server_method(
            'local_entity_message',
            # {'m': RpcIndexer.send_rpc_index(method_name), 'p': parameters}
            {'m': method_name, 'p': parameters}
        )

    def call_client_method(self, method_name, parameters):
        # if not gr.bind_entity:
        #     return
        # if gr.IS_RECORD_PLAY:
        #     return
        # if not self.dungeon:
        #     raise StandardError()
        # if const.PRINT_PROTO:
        #     if method_name not in ('move_to', 'CompMovement.get_rtt_info', 'CompLocalEntityRouter.route', 'ride_move_to'):
        #         if gr.is_client:
        #             gr.logger.warn("[aoi c=>s]: %s", method_name)

        # gr.bind_entity.call_server_method(
        self.bind_entity.call_client_method(
            'local_entity_message',
            # {'m': RpcIndexer.send_rpc_index(method_name), 'p': parameters}
            {'m': method_name, 'p': parameters}
        )
