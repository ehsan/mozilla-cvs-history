INSERT INTO test_case_run_status (name, sortkey) VALUES ('IDLE', 1)
INSERT INTO test_case_run_status (name, sortkey) VALUES ('PASSED', 2)
INSERT INTO test_case_run_status (name, sortkey) VALUES ('FAILED', 3)
INSERT INTO test_case_run_status (name, sortkey) VALUES ('RUNNING', 4)
INSERT INTO test_case_run_status (name, sortkey) VALUES ('PAUSED', 5)
INSERT INTO test_case_run_status (name, sortkey) VALUES ('BLOCKED', 6)
INSERT INTO test_case_status (name) VALUES ('PROPOSED')
INSERT INTO test_case_status (name) VALUES ('CONFIRMED')
INSERT INTO test_case_status (name) VALUES ('DISABLED')
INSERT INTO test_plan_types (name) VALUES ('Unit')
INSERT INTO test_plan_types (name) VALUES ('Integration')
INSERT INTO test_plan_types (name) VALUES ('Function')
INSERT INTO test_plan_types (name) VALUES ('System')
INSERT INTO test_plan_types (name) VALUES ('Acceptance')
INSERT INTO test_plan_types (name) VALUES ('Installation')
INSERT INTO test_plan_types (name) VALUES ('Performance')
INSERT INTO test_plan_types (name) VALUES ('Product')
INSERT INTO test_plan_types (name) VALUES ('Interoperability')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('isactive', 'Archived', 'test_plans')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('name', 'Plan Name', 'test_plans')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('type_id', 'Plan Type', 'test_plans')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('case_status_id', 'Case Status', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('category_id', 'Category', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('priority_id', 'Priority', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('summary', 'Run Summary', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('isautomated', 'Automated', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('alias', 'Alias', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('requirement', 'Requirement', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('script', 'Script', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('arguments', 'Argument', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('product_id', 'Product', 'test_plans')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('default_product_version', 'Default Product Version', 'test_plans')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('environment_id', 'Environment', 'test_runs')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('product_version', 'Product Version', 'test_runs')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('build_id', 'Default Build', 'test_runs')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('plan_text_version', 'Plan Text Version', 'test_runs')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('manager_id', 'Manager', 'test_runs')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('default_tester_id', 'Default Tester', 'test_cases')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('stop_date', 'Stop Date', 'test_runs')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('summary', 'Run Summary', 'test_runs')
INSERT INTO test_fielddefs (name, description, table_name) VALUES ('notes', 'Notes', 'test_runs')
