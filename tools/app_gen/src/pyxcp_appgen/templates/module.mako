<%inherit file="project.mako"/>

<%def name="module_module(module)">
/begin MODULE ${module['name']} ""
% if module.get('includes'):
% for inc in module['includes']:
/include "${inc}"
% endfor
% endif

% if module.get('mod_common'):
/begin MOD_COMMON ""
    % for k,v in module['mod_common'].items():
    %   if k.startswith('alignment'):
    ${k.upper().replace('_',' ')} ${v}
    %   else:
    ${k.upper()} ${v}
    %   endif
    % endfor
/end MOD_COMMON
% endif

% for c in module.get('characteristics', []):
/begin CHARACTERISTIC ${c['name']} "${c.get('comment','')}"
    VALUE
    ${c.get('value','0')}
    ${c_to_asam_type(c.get('type',''))}
    0
    NO_COMPU_METHOD
    ${c.get('min','0')}
    ${c.get('max','0')}
    PHYS_UNIT ${'"' + c.get('phys_unit', "") + '"'}
/end CHARACTERISTIC

% endfor

% for m in module.get('measurements', []):
${self.measurement_block(m)}
% endfor

% for g in module.get('groups', []):
/begin GROUP ${g['name']} "${g.get('comment','')}" ${ "ROOT" if g.get('root') else "" }
    % if g.get('ref_characteristics'):
    /begin REF_CHARACTERISTIC ${ " ".join(g['ref_characteristics']) }
    /end REF_CHARACTERISTIC
    % endif
    % if g.get('ref_measurements'):
    /begin REF_MEASUREMENT ${ " ".join(g['ref_measurements']) }
    /end REF_MEASUREMENT
    % endif
    % if g.get('sub_groups'):
    /begin SUB_GROUP ${ " ".join(g['sub_groups']) }
    /end SUB_GROUP
    % endif
/end GROUP

% endfor

% if module.get('mod_par'):
/begin MOD_PAR ""
    EPK "${module['mod_par'].get('epk','')}" ADDR_EPK ${module['mod_par'].get('addr_epk','0')}
    % for mem in module.get('memory_segments', []):
    /begin MEMORY_SEGMENT ${mem['name']} "" DATA ${mem['type']} INTERN ${mem['addr']} ${mem['size']} -1 -1 -1 -1 -1
        % if mem.get('if_data') and mem['if_data'].get('XCP'):
       /begin IF_DATA XCP
           % for seg in mem['if_data']['XCP'].get('segments', []):
           /begin SEGMENT ${seg.get('index',0)} 2 0 0 0
               % if seg.get('checksum'):
               /begin CHECKSUM ${seg['checksum']['type']}
                    MAX_BLOCK_SIZE ${seg['checksum']['max_block_size']}
                    EXTERNAL_FUNCTION ""
               /end CHECKSUM
               % endif
               % for p in seg.get('pages', []):
               /begin PAGE ${p.get('page',0)}
                    ECU_ACCESS_DONT_CARE
                    XCP_READ_ACCESS_DONT_CARE
                    ${p.get('xcp_write_access',"XCP_WRITE_ACCESS_DONT_CARE")}
               /end PAGE
               % endfor
           /end SEGMENT
           % endfor
       /end IF_DATA
       % endif
    /end MEMORY_SEGMENT
    % endfor
/end MOD_PAR
% endif

/end MODULE
</%def>

<%def name="measurement_block(m)">
/begin MEASUREMENT ${m['name']} "${m.get('comment','')}"
    ${c_to_asam_type(m.get('type',''))}
    ${ m.get('compu_method','NO_COMPU_METHOD') }
    0
    0
    ${m.get('min','0')}
    ${m.get('max','0')}
    ${ "ECU_ADDRESS " + hex(symbol_addresses.get(m['name'], -1)) }
    ${ "ECU_ADDRESS_EXTENSION " + "0" }
    PHYS_UNIT ${'"' + m.get('phys_unit', "") + '"'}
    % if m.get('if_data') and m['if_data'].get('XCP'):
    /begin IF_DATA XCP
        % if m['if_data']['XCP'].get('daq_event'):
        /begin DAQ_EVENT ${m['if_data']['XCP']['daq_event'].get('type','')}
            % if m['if_data']['XCP']['daq_event'].get('default_event_list'):
            /begin DEFAULT_EVENT_LIST
                EVENT ${m['if_data']['XCP']['daq_event']['default_event_list'][0]}
            /end DEFAULT_EVENT_LIST
            % elif m['if_data']['XCP']['daq_event'].get('event') is not None:
            % for ev in ([m['if_data']['XCP']['daq_event'].get('event')]):
            /begin FIXED_EVENT_LIST
                EVENT ${ev}
            /end FIXED_EVENT_LIST
            % endfor
            % endif
        /end DAQ_EVENT
        % endif
    /end IF_DATA
    % endif
/end MEASUREMENT
</%def>