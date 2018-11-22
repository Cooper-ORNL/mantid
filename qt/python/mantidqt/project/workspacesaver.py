# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2017 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantidqt package
#

from mantid.api import AnalysisDataService as ADS, IMDEventWorkspace
from mantid.dataobjects import MDHistoWorkspace, MaskWorkspace
from mantid.simpleapi import SaveMask, SaveMD, SaveNexusProcessed


# Static method to get all workspaces to save
def _get_all_workspaces_to_save():
    """
    Get's all the workspaces that should be saved (Allows further expansion later)
    :return: List; Workspaces
    """
    return ADS.getObjectNames()


class WorkspaceSaver(object):
    def __init__(self, directory):
        """

        :param directory:
        """
        self.directory = directory
        self.output_list = []

    def save_workspaces(self, workspaces_to_save=None):
        """
        Use the private method _get_workspaces_to_save to get a list of workspaces that are present in the ADS to save
        to the directory that was passed at object creation time, it will also add each of them to the output_list
        private instance variable on the WorkspaceSaver class.
        :param workspaces_to_save:
        """

        # Handle getting here and nothing has been given passed
        if workspaces_to_save is None:
            return

        for workspace_name in workspaces_to_save:
            # Get the workspace from the ADS
            workspace = ADS.retrieve(workspace_name)
            place_to_save_workspace = self.directory + '/' + workspace_name

            if isinstance(workspace, MDHistoWorkspace) or isinstance(workspace, IMDEventWorkspace):
                # Save normally using SaveMD
                SaveMD(InputWorkspace=workspace_name, Filename=place_to_save_workspace)
            else:
                # Save normally using SaveNexusProcessed
                SaveNexusProcessed(InputWorkspace=workspace_name, Filename=place_to_save_workspace)

            self.output_list.append(workspace_name)

    def get_output_list(self):
        """
        Get the output_list
        :return: List; String list of the workspaces that were saved
        """
        return self.output_list
