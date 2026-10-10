<%!
  def fmt_list(lst):
      return "\n".join(lst) if lst else ""
%>
ASAP2_VERSION ${project['asap2_version'][0]} ${project['asap2_version'][1]}
/begin PROJECT ${project['name']} ""
/begin HEADER ""
    VERSION "${project['header']['version']}"
    PROJECT_NO ${project['header']['project_no']}
/end HEADER
${self.module_module(project['module'])}
/end PROJECT
