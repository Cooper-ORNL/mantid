default_dict = {'first_good_time': 0.1, 'last_good_time': 15, 'forward_group': 'fwd', 'backward_group': 'bwd', 'input_workspace': '',
                'phase_quad_input_workspace': '', 'phase_table_for_phase_quad': ''}


class PhaseTableContext(object):
    def __init__(self):
        self.options_dict = default_dict.copy()
        self.phase_tables = []

    def add_phase_table(self, name):
        if name not in self.phase_tables:
            self.phase_tables.append(name)

    def get_phase_table_list(self, instrument):
        return [phase_table for phase_table in self.phase_tables if instrument in phase_table]
