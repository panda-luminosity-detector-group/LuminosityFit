import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

from python.util.FileListBuncher import FileListBuncher

"""
Todo: auto generated, make real tests!
"""
class TestFileListBuncher(unittest.TestCase):
    def setUp(self):
        self.buncher = FileListBuncher()
    
    @patch("pathlib.Path.is_dir")
    @patch("pathlib.Path.iterdir")
    def test_collect_list_of_directories(self, mock_iterdir, mock_is_dir):
        # Mock functions
        mock_is_dir.return_value = True
        mock_iterdir.return_value = [Path("sub_directory")]

        # Run the function
        self.buncher.collect_list_of_directories(Path("."))

        # Assertions
        self.assertEqual(len(self.buncher._dirs), 1)
        self.assertEqual(self.buncher._dirs[0], Path("sub_directory"))

    @patch("builtins.open")
    def test_create_file_list_file(self, mock_open):
        # Prepare data
        mock_file = MagicMock()
        mock_open.return_value.__enter__.return_value = mock_file

        # Run function
        self.buncher.create_file_list_file(Path("./output.txt"), ["abc", "def"])

        # Assertions
        calls = [call('abc\n'), call('def\n')]
        mock_file.write.assert_has_calls(calls)

    @patch("lumifit.general.getGoodFiles")
    @patch.object(FileListBuncher, "create_file_list_file")
    def test_make_file_list_bunches(self, mock_create_file, mock_getGoodFiles):
        # Mock functions
        mock_getGoodFiles.return_value = (['file_1', 'file_2', 'file_3'], None)

        # Run the function
        self.buncher.files_per_bunch = 2
        self.buncher.make_file_list_bunches(Path("."))

        # Assertions
        self.assertEqual(mock_create_file.call_count, 2)

    @patch.object(FileListBuncher, "collect_list_of_directories")
    @patch.object(FileListBuncher, "make_file_list_bunches")
    def test_run(self, mock_make_list, mock_collect_dir):
        # Mock functions
        self.buncher._dirs = [Path(".")]

        # Run the function
        self.buncher.run()

        # Assertions
        self.assertTrue(mock_collect_dir.called)
        self.assertTrue(mock_make_list.called)
        

if __name__ == "__main__":
    unittest.main()
