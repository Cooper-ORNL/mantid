# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2017 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantidqt package
#
import unittest

from os import listdir
from os.path import isdir, expanduser
from shutil import rmtree

from mantid.api import AnalysisDataService as ADS
from mantid.simpleapi import CreateSampleWorkspace
from mantidqt.project import projectsaver


working_directory = expanduser("~") + "/project_saver_test"


class ProjectSaverTest(unittest.TestCase):
    def tearDown(self):
        ADS.clear()
        if isdir(working_directory):
            rmtree(working_directory)

    def test_only_one_workspace_saving(self):
        ws1_name = "ws1"
        ADS.addOrReplace(ws1_name, CreateSampleWorkspace(OutputWorkspace=ws1_name))
        project_saver = projectsaver.ProjectSaver(save_location=working_directory)
        file_name = working_directory + "/" + projectsaver.ENCODED_FILE_NAME
        saved_file = "{\"Interfaces\": {}, \"Workspaces\": [\"ws1\"]}"

        project_saver.save_project(workspace_to_save=[ws1_name])

        # Check project file is saved correctly
        f = open(file_name, "r")
        self.assertEqual(f.read(), saved_file)

        # Check workspace is saved
        list_of_files = listdir(working_directory)
        self.assertEqual(len(list_of_files), 2)
        self.assertTrue(projectsaver.ENCODED_FILE_NAME in list_of_files)
        self.assertTrue(ws1_name in list_of_files)

    def test_only_multiple_workspaces_saving(self):
        ws1_name = "ws1"
        ws2_name = "ws2"
        ws3_name = "ws3"
        ws4_name = "ws4"
        ws5_name = "ws5"
        CreateSampleWorkspace(OutputWorkspace=ws1_name)
        CreateSampleWorkspace(OutputWorkspace=ws2_name)
        CreateSampleWorkspace(OutputWorkspace=ws3_name)
        CreateSampleWorkspace(OutputWorkspace=ws4_name)
        CreateSampleWorkspace(OutputWorkspace=ws5_name)
        project_saver = projectsaver.ProjectSaver(save_location=working_directory)
        file_name = working_directory + "/" + projectsaver.ENCODED_FILE_NAME
        saved_file = "{\"Interfaces\": {}, \"Workspaces\": [\"ws1\", \"ws2\", \"ws3\", \"ws4\", \"ws5\"]}"

        project_saver.save_project(workspace_to_save=[ws1_name, ws2_name, ws3_name, ws4_name, ws5_name])

        # Check project file is saved correctly
        f = open(file_name, "r")
        self.assertEqual(f.read(), saved_file)

        # Check workspace is saved
        list_of_files = listdir(working_directory)
        self.assertEqual(len(list_of_files), 6)
        self.assertTrue(projectsaver.ENCODED_FILE_NAME in list_of_files)
        self.assertTrue(ws1_name in list_of_files)
        self.assertTrue(ws2_name in list_of_files)
        self.assertTrue(ws3_name in list_of_files)
        self.assertTrue(ws4_name in list_of_files)
        self.assertTrue(ws5_name in list_of_files)

    def test_only_saving_one_workspace_when_multiple_are_present_in_the_ADS(self):
        ws1_name = "ws1"
        ws2_name = "ws2"
        ws3_name = "ws3"
        CreateSampleWorkspace(OutputWorkspace=ws1_name)
        CreateSampleWorkspace(OutputWorkspace=ws2_name)
        CreateSampleWorkspace(OutputWorkspace=ws3_name)
        project_saver = projectsaver.ProjectSaver(save_location=working_directory)
        file_name = working_directory + "/" + projectsaver.ENCODED_FILE_NAME
        saved_file = "{\"Interfaces\": {}, \"Workspaces\": [\"ws1\"]}"

        project_saver.save_project(workspace_to_save=[ws1_name])

        # Check project file is saved correctly
        f = open(file_name, "r")
        self.assertEqual(f.read(), saved_file)

        # Check workspace is saved
        list_of_files = listdir(working_directory)
        self.assertEqual(len(list_of_files), 2)
        self.assertTrue(projectsaver.ENCODED_FILE_NAME in list_of_files)
        self.assertTrue(ws1_name in list_of_files)

    #def test_only_one_interface_saving(self):

    #def test_only_multiple_interfaces_saving(self):

    #def test_one_workspace_and_one_interface_saving(self):

    #def test_multiple_workspaces_and_multiple_interfaces(self):

    #def test_get_encoders_retrieves_correct_encoder(self):

    #def test_encode_interfaces_on_one_interface(self):

    #def test_encode_interfaces_on_multiple_interfaces(self):


class ProjectWriterTest(unittest.TestCase):
    def tearDown(self):
        ADS.clear()
        if isdir(working_directory):
            rmtree(working_directory)

    def test_create_out_string_on_smaller_number_of_interfaces(self):
        small_dict = {"Workspaces": {}, "Interfaces": {"interface1": {"value1": 2, "value2": 3},
                                                       "interface2": {"value3": 4, "value4": 5}}}
        json_dict = "{\"Interfaces\": {\"interface1\": {\"value2\": 3, \"value1\": 2}, \"interface2\": {\"value4\":" \
                    " 5, \"value3\": 4}}, \"Workspaces\": {}}"

        output = projectsaver._create_out_string_json(small_dict)
        self.assertEqual(output, json_dict)

    def test_create_out_string_on_larger_number_of_interfaces(self):
        large_dict = {"Workspaces": {}, "Interfaces": {"interface1": {"value1": 2, "value2": 3},
                                                       "interface2": {"value3": 4, "value4": 5},
                                                       "interface3": {"value1": 2, "value2": 3},
                                                       "interface4": {"value3": 4, "value4": 5},
                                                       "interface5": {"value1": 2, "value2": 3},
                                                       "interface6": {"value3": 4, "value4": 5}}}
        json_dict = "{\"Interfaces\": {\"interface1\": {\"value2\": 3, \"value1\": 2}, \"interface3\": {\"value2\": " \
                    "3, \"value1\": 2}, \"interface2\": {\"value4\": 5, \"value3\": 4}, \"interface5\": {\"value2\":" \
                    " 3, \"value1\": 2}, \"interface4\": {\"value4\": 5, \"value3\": 4}, \"interface6\": {\"value4\":" \
                    " 5, \"value3\": 4}}, \"Workspaces\": {}}"

        output = projectsaver._create_out_string_json(large_dict)
        self.assertEqual(output, json_dict)

    def test_create_out_string_on_empty_dict(self):
        empty_dict = {"Workspaces": {}, "Interfaces": {}}
        json_dict = "{\"Interfaces\": {}, \"Workspaces\": {}}"

        output = projectsaver._create_out_string_json(empty_dict)
        self.assertEqual(output, json_dict)

    def test_create_workspace_only_json_string(self):
        workspaces_dict = {"Workspaces": ["ws1", "ws2", "ws3", "ws4"], "Interfaces": {}}
        json_list = "{\"Interfaces\": {}, \"Workspaces\": [\"ws1\", \"ws2\", \"ws3\", \"ws4\"]}"

        output = projectsaver._create_out_string_json(workspaces_dict)
        self.assertEqual(output, json_list)

    def test_write_out_on_just_dicts(self):
        workspace_list = []
        small_dict = {"interface1": {"value1": 2, "value2": 3}, "interface2": {"value3": 4, "value4": 5}}
        project_writer = projectsaver.ProjectWriter(small_dict, working_directory, workspace_list)
        file_name = working_directory + "/" + projectsaver.ENCODED_FILE_NAME
        saved_file = "{\"Interfaces\": {\"interface1\": {\"value2\": 3, \"value1\": 2}, \"interface2\": {\"value4\"" \
                     ": 5, \"value3\": 4}}, \"Workspaces\": []}"

        project_writer.write_out()

        f = open(file_name, "r")
        self.assertEqual(f.read(), saved_file)

    def test_write_out_on_just_workspaces(self):
        workspace_list = ["ws1", "ws2", "ws3", "ws4"]
        small_dict = {}
        project_writer = projectsaver.ProjectWriter(small_dict, working_directory, workspace_list)
        file_name = working_directory + "/" + projectsaver.ENCODED_FILE_NAME
        saved_file = "{\"Interfaces\": {}, \"Workspaces\": [\"ws1\", \"ws2\", \"ws3\", \"ws4\"]}"

        project_writer.write_out()

        f = open(file_name, "r")
        self.assertEqual(f.read(), saved_file)

    def test_write_out_on_both_workspaces_and_dicts(self):
        workspace_list = ["ws1", "ws2", "ws3", "ws4"]
        small_dict = {"interface1": {"value1": 2, "value2": 3}, "interface2": {"value3": 4, "value4": 5}}
        project_writer = projectsaver.ProjectWriter(small_dict, working_directory, workspace_list)
        file_name = working_directory + "/" + projectsaver.ENCODED_FILE_NAME
        saved_file = "{\"Interfaces\": {\"interface1\": {\"value2\": 3, \"value1\": 2}, \"interface2\": {\"value4\":" \
                     " 5, \"value3\": 4}}, \"Workspaces\": [\"ws1\", \"ws2\", \"ws3\", \"ws4\"]}"
        project_writer.write_out()

        f = open(file_name, "r")
        self.assertEqual(f.read(), saved_file)