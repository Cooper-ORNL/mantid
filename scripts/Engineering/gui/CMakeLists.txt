# Tests for ISIS Engineering Diffraction

set(TEST_PY_FILES
    # Common
    engineering_diffraction/tabs/common/test/test_vanadium_corrections.py
    # Cropping
    engineering_diffraction/tabs/common/cropping/test/test_cropping_model.py
    engineering_diffraction/tabs/common/cropping/test/test_cropping_presenter.py
    # Settings
    engineering_diffraction/settings/test/test_settings_helper.py
    engineering_diffraction/settings/test/test_settings_model.py
    engineering_diffraction/settings/test/test_settings_presenter.py
    # Calibration
    engineering_diffraction/tabs/calibration/test/test_calib_model.py
    engineering_diffraction/tabs/calibration/test/test_calib_presenter.py
    # Fitting
    engineering_diffraction/tabs/fitting/data_handling/test/test_data_model.py
    engineering_diffraction/tabs/fitting/data_handling/test/test_data_presenter.py
    # Focus
    engineering_diffraction/tabs/focus/test/test_focus_model.py
    engineering_diffraction/tabs/focus/test/test_focus_presenter.py
)

check_tests_valid(${CMAKE_CURRENT_SOURCE_DIR} ${TEST_PY_FILES})

pyunittest_add_test(${CMAKE_CURRENT_SOURCE_DIR} python.EngineeringDiffraction
                    ${TEST_PY_FILES})