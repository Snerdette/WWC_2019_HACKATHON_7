import React, {Component} from 'react';
import {bindActionCreators} from 'redux';
import {connect} from 'react-redux';
import {selectProbe} from "../actions/index";

class ProbeList extends Component {

    renderList() {
        return this.props.probes.map((probe) => {
            return (
                <div>
                    <table cellSpacing="10">
                        <thead>
                        <tr>
                            <th>ID</th>
                            <th>Location</th>
                            <th>Status</th>
                            <th>Current Methane Level</th>
                            <th>Install Date</th>
                        </tr>
                        </thead>
                        <tbody>
                        <tr>
                            <td onClick={() => this.props.selectProbe(probe)}>{probe.id}</td>
                            <td onClick={() => this.props.selectProbe(probe)}>{probe.location}</td>
                            <td onClick={() => this.props.selectProbe(probe)}>{probe.status}</td>
                            <td onClick={() => this.props.selectProbe(probe)}>{probe.methaneLevel}</td>
                            <td onClick={() => this.props.selectProbe(probe)}>{probe.installDate}</td>
                        </tr>
                        </tbody>
                    </table>
                </div>

            );
        });
    }

    render () {
        return (
            <ul>
                {this.renderList()}
            </ul>
        );
    }
}

function mapStateToProps(state) {
    return {
        probes: state.probes
    };
}

// Get actions and pass them as props to to UserList
//      > now UserList has this.props.selectUser
function matchDispatchToProps(dispatch){
    return bindActionCreators({selectProbe: selectProbe}, dispatch);
}

// We don't want to return the plain UserList (component) anymore, we want to return the smart Container
//      > UserList is now aware of state and actions
export default connect(mapStateToProps, matchDispatchToProps)(ProbeList);

let alignMiddleStyle = {
    verticalAlign: 'middle'
};