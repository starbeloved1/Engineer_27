# generated from rosidl_generator_py/resource/_idl.py.em
# with input from arm_planning:action/PlanMotion.idl
# generated code does not contain a copyright notice


# Import statements for member types

import builtins  # noqa: E402, I100

import math  # noqa: E402, I100

# Member 'joint_positions'
import numpy  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_PlanMotion_Goal(type):
    """Metaclass of message 'PlanMotion_Goal'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
        'JOINT': 0,
        'POSE': 1,
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_Goal')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__action__plan_motion__goal
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__action__plan_motion__goal
            cls._CONVERT_TO_PY = module.convert_to_py_msg__action__plan_motion__goal
            cls._TYPE_SUPPORT = module.type_support_msg__action__plan_motion__goal
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__action__plan_motion__goal

            from geometry_msgs.msg import PoseStamped
            if PoseStamped.__class__._TYPE_SUPPORT is None:
                PoseStamped.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
            'JOINT': cls.__constants['JOINT'],
            'POSE': cls.__constants['POSE'],
        }

    @property
    def JOINT(self):
        """Message constant 'JOINT'."""
        return Metaclass_PlanMotion_Goal.__constants['JOINT']

    @property
    def POSE(self):
        """Message constant 'POSE'."""
        return Metaclass_PlanMotion_Goal.__constants['POSE']


class PlanMotion_Goal(metaclass=Metaclass_PlanMotion_Goal):
    """
    Message class 'PlanMotion_Goal'.

    Constants:
      JOINT
      POSE
    """

    __slots__ = [
        '_target_type',
        '_joint_positions',
        '_target_pose',
        '_execute',
        '_velocity_scaling',
        '_acceleration_scaling',
        '_planning_time',
    ]

    _fields_and_field_types = {
        'target_type': 'uint8',
        'joint_positions': 'double[6]',
        'target_pose': 'geometry_msgs/PoseStamped',
        'execute': 'boolean',
        'velocity_scaling': 'double',
        'acceleration_scaling': 'double',
        'planning_time': 'double',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.BasicType('uint8'),  # noqa: E501
        rosidl_parser.definition.Array(rosidl_parser.definition.BasicType('double'), 6),  # noqa: E501
        rosidl_parser.definition.NamespacedType(['geometry_msgs', 'msg'], 'PoseStamped'),  # noqa: E501
        rosidl_parser.definition.BasicType('boolean'),  # noqa: E501
        rosidl_parser.definition.BasicType('double'),  # noqa: E501
        rosidl_parser.definition.BasicType('double'),  # noqa: E501
        rosidl_parser.definition.BasicType('double'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.target_type = kwargs.get('target_type', int())
        if 'joint_positions' not in kwargs:
            self.joint_positions = numpy.zeros(6, dtype=numpy.float64)
        else:
            self.joint_positions = kwargs.get('joint_positions')
        from geometry_msgs.msg import PoseStamped
        self.target_pose = kwargs.get('target_pose', PoseStamped())
        self.execute = kwargs.get('execute', bool())
        self.velocity_scaling = kwargs.get('velocity_scaling', float())
        self.acceleration_scaling = kwargs.get('acceleration_scaling', float())
        self.planning_time = kwargs.get('planning_time', float())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.target_type != other.target_type:
            return False
        if any(self.joint_positions != other.joint_positions):
            return False
        if self.target_pose != other.target_pose:
            return False
        if self.execute != other.execute:
            return False
        if self.velocity_scaling != other.velocity_scaling:
            return False
        if self.acceleration_scaling != other.acceleration_scaling:
            return False
        if self.planning_time != other.planning_time:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def target_type(self):
        """Message field 'target_type'."""
        return self._target_type

    @target_type.setter
    def target_type(self, value):
        if __debug__:
            assert \
                isinstance(value, int), \
                "The 'target_type' field must be of type 'int'"
            assert value >= 0 and value < 256, \
                "The 'target_type' field must be an unsigned integer in [0, 255]"
        self._target_type = value

    @builtins.property
    def joint_positions(self):
        """Message field 'joint_positions'."""
        return self._joint_positions

    @joint_positions.setter
    def joint_positions(self, value):
        if isinstance(value, numpy.ndarray):
            assert value.dtype == numpy.float64, \
                "The 'joint_positions' numpy.ndarray() must have the dtype of 'numpy.float64'"
            assert value.size == 6, \
                "The 'joint_positions' numpy.ndarray() must have a size of 6"
            self._joint_positions = value
            return
        if __debug__:
            from collections.abc import Sequence
            from collections.abc import Set
            from collections import UserList
            from collections import UserString
            assert \
                ((isinstance(value, Sequence) or
                  isinstance(value, Set) or
                  isinstance(value, UserList)) and
                 not isinstance(value, str) and
                 not isinstance(value, UserString) and
                 len(value) == 6 and
                 all(isinstance(v, float) for v in value) and
                 all(not (val < -1.7976931348623157e+308 or val > 1.7976931348623157e+308) or math.isinf(val) for val in value)), \
                "The 'joint_positions' field must be a set or sequence with length 6 and each value of type 'float' and each double in [-179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000, 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000]"
        self._joint_positions = numpy.array(value, dtype=numpy.float64)

    @builtins.property
    def target_pose(self):
        """Message field 'target_pose'."""
        return self._target_pose

    @target_pose.setter
    def target_pose(self, value):
        if __debug__:
            from geometry_msgs.msg import PoseStamped
            assert \
                isinstance(value, PoseStamped), \
                "The 'target_pose' field must be a sub message of type 'PoseStamped'"
        self._target_pose = value

    @builtins.property
    def execute(self):
        """Message field 'execute'."""
        return self._execute

    @execute.setter
    def execute(self, value):
        if __debug__:
            assert \
                isinstance(value, bool), \
                "The 'execute' field must be of type 'bool'"
        self._execute = value

    @builtins.property
    def velocity_scaling(self):
        """Message field 'velocity_scaling'."""
        return self._velocity_scaling

    @velocity_scaling.setter
    def velocity_scaling(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'velocity_scaling' field must be of type 'float'"
            assert not (value < -1.7976931348623157e+308 or value > 1.7976931348623157e+308) or math.isinf(value), \
                "The 'velocity_scaling' field must be a double in [-1.7976931348623157e+308, 1.7976931348623157e+308]"
        self._velocity_scaling = value

    @builtins.property
    def acceleration_scaling(self):
        """Message field 'acceleration_scaling'."""
        return self._acceleration_scaling

    @acceleration_scaling.setter
    def acceleration_scaling(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'acceleration_scaling' field must be of type 'float'"
            assert not (value < -1.7976931348623157e+308 or value > 1.7976931348623157e+308) or math.isinf(value), \
                "The 'acceleration_scaling' field must be a double in [-1.7976931348623157e+308, 1.7976931348623157e+308]"
        self._acceleration_scaling = value

    @builtins.property
    def planning_time(self):
        """Message field 'planning_time'."""
        return self._planning_time

    @planning_time.setter
    def planning_time(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'planning_time' field must be of type 'float'"
            assert not (value < -1.7976931348623157e+308 or value > 1.7976931348623157e+308) or math.isinf(value), \
                "The 'planning_time' field must be a double in [-1.7976931348623157e+308, 1.7976931348623157e+308]"
        self._planning_time = value


# Import statements for member types

# already imported above
# import builtins

# already imported above
# import math

# Member 'selected_joint_positions'
# already imported above
# import numpy

# already imported above
# import rosidl_parser.definition


class Metaclass_PlanMotion_Result(type):
    """Metaclass of message 'PlanMotion_Result'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_Result')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__action__plan_motion__result
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__action__plan_motion__result
            cls._CONVERT_TO_PY = module.convert_to_py_msg__action__plan_motion__result
            cls._TYPE_SUPPORT = module.type_support_msg__action__plan_motion__result
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__action__plan_motion__result

            from moveit_msgs.msg import RobotTrajectory
            if RobotTrajectory.__class__._TYPE_SUPPORT is None:
                RobotTrajectory.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class PlanMotion_Result(metaclass=Metaclass_PlanMotion_Result):
    """Message class 'PlanMotion_Result'."""

    __slots__ = [
        '_success',
        '_error_code',
        '_message',
        '_trajectory',
        '_selected_joint_positions',
        '_planning_time',
        '_joint_path_length',
        '_validation_samples',
        '_ik_candidates',
    ]

    _fields_and_field_types = {
        'success': 'boolean',
        'error_code': 'int32',
        'message': 'string',
        'trajectory': 'moveit_msgs/RobotTrajectory',
        'selected_joint_positions': 'double[6]',
        'planning_time': 'double',
        'joint_path_length': 'double',
        'validation_samples': 'uint32',
        'ik_candidates': 'uint32',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.BasicType('boolean'),  # noqa: E501
        rosidl_parser.definition.BasicType('int32'),  # noqa: E501
        rosidl_parser.definition.UnboundedString(),  # noqa: E501
        rosidl_parser.definition.NamespacedType(['moveit_msgs', 'msg'], 'RobotTrajectory'),  # noqa: E501
        rosidl_parser.definition.Array(rosidl_parser.definition.BasicType('double'), 6),  # noqa: E501
        rosidl_parser.definition.BasicType('double'),  # noqa: E501
        rosidl_parser.definition.BasicType('double'),  # noqa: E501
        rosidl_parser.definition.BasicType('uint32'),  # noqa: E501
        rosidl_parser.definition.BasicType('uint32'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.success = kwargs.get('success', bool())
        self.error_code = kwargs.get('error_code', int())
        self.message = kwargs.get('message', str())
        from moveit_msgs.msg import RobotTrajectory
        self.trajectory = kwargs.get('trajectory', RobotTrajectory())
        if 'selected_joint_positions' not in kwargs:
            self.selected_joint_positions = numpy.zeros(6, dtype=numpy.float64)
        else:
            self.selected_joint_positions = kwargs.get('selected_joint_positions')
        self.planning_time = kwargs.get('planning_time', float())
        self.joint_path_length = kwargs.get('joint_path_length', float())
        self.validation_samples = kwargs.get('validation_samples', int())
        self.ik_candidates = kwargs.get('ik_candidates', int())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.success != other.success:
            return False
        if self.error_code != other.error_code:
            return False
        if self.message != other.message:
            return False
        if self.trajectory != other.trajectory:
            return False
        if any(self.selected_joint_positions != other.selected_joint_positions):
            return False
        if self.planning_time != other.planning_time:
            return False
        if self.joint_path_length != other.joint_path_length:
            return False
        if self.validation_samples != other.validation_samples:
            return False
        if self.ik_candidates != other.ik_candidates:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def success(self):
        """Message field 'success'."""
        return self._success

    @success.setter
    def success(self, value):
        if __debug__:
            assert \
                isinstance(value, bool), \
                "The 'success' field must be of type 'bool'"
        self._success = value

    @builtins.property
    def error_code(self):
        """Message field 'error_code'."""
        return self._error_code

    @error_code.setter
    def error_code(self, value):
        if __debug__:
            assert \
                isinstance(value, int), \
                "The 'error_code' field must be of type 'int'"
            assert value >= -2147483648 and value < 2147483648, \
                "The 'error_code' field must be an integer in [-2147483648, 2147483647]"
        self._error_code = value

    @builtins.property
    def message(self):
        """Message field 'message'."""
        return self._message

    @message.setter
    def message(self, value):
        if __debug__:
            assert \
                isinstance(value, str), \
                "The 'message' field must be of type 'str'"
        self._message = value

    @builtins.property
    def trajectory(self):
        """Message field 'trajectory'."""
        return self._trajectory

    @trajectory.setter
    def trajectory(self, value):
        if __debug__:
            from moveit_msgs.msg import RobotTrajectory
            assert \
                isinstance(value, RobotTrajectory), \
                "The 'trajectory' field must be a sub message of type 'RobotTrajectory'"
        self._trajectory = value

    @builtins.property
    def selected_joint_positions(self):
        """Message field 'selected_joint_positions'."""
        return self._selected_joint_positions

    @selected_joint_positions.setter
    def selected_joint_positions(self, value):
        if isinstance(value, numpy.ndarray):
            assert value.dtype == numpy.float64, \
                "The 'selected_joint_positions' numpy.ndarray() must have the dtype of 'numpy.float64'"
            assert value.size == 6, \
                "The 'selected_joint_positions' numpy.ndarray() must have a size of 6"
            self._selected_joint_positions = value
            return
        if __debug__:
            from collections.abc import Sequence
            from collections.abc import Set
            from collections import UserList
            from collections import UserString
            assert \
                ((isinstance(value, Sequence) or
                  isinstance(value, Set) or
                  isinstance(value, UserList)) and
                 not isinstance(value, str) and
                 not isinstance(value, UserString) and
                 len(value) == 6 and
                 all(isinstance(v, float) for v in value) and
                 all(not (val < -1.7976931348623157e+308 or val > 1.7976931348623157e+308) or math.isinf(val) for val in value)), \
                "The 'selected_joint_positions' field must be a set or sequence with length 6 and each value of type 'float' and each double in [-179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000, 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000]"
        self._selected_joint_positions = numpy.array(value, dtype=numpy.float64)

    @builtins.property
    def planning_time(self):
        """Message field 'planning_time'."""
        return self._planning_time

    @planning_time.setter
    def planning_time(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'planning_time' field must be of type 'float'"
            assert not (value < -1.7976931348623157e+308 or value > 1.7976931348623157e+308) or math.isinf(value), \
                "The 'planning_time' field must be a double in [-1.7976931348623157e+308, 1.7976931348623157e+308]"
        self._planning_time = value

    @builtins.property
    def joint_path_length(self):
        """Message field 'joint_path_length'."""
        return self._joint_path_length

    @joint_path_length.setter
    def joint_path_length(self, value):
        if __debug__:
            assert \
                isinstance(value, float), \
                "The 'joint_path_length' field must be of type 'float'"
            assert not (value < -1.7976931348623157e+308 or value > 1.7976931348623157e+308) or math.isinf(value), \
                "The 'joint_path_length' field must be a double in [-1.7976931348623157e+308, 1.7976931348623157e+308]"
        self._joint_path_length = value

    @builtins.property
    def validation_samples(self):
        """Message field 'validation_samples'."""
        return self._validation_samples

    @validation_samples.setter
    def validation_samples(self, value):
        if __debug__:
            assert \
                isinstance(value, int), \
                "The 'validation_samples' field must be of type 'int'"
            assert value >= 0 and value < 4294967296, \
                "The 'validation_samples' field must be an unsigned integer in [0, 4294967295]"
        self._validation_samples = value

    @builtins.property
    def ik_candidates(self):
        """Message field 'ik_candidates'."""
        return self._ik_candidates

    @ik_candidates.setter
    def ik_candidates(self, value):
        if __debug__:
            assert \
                isinstance(value, int), \
                "The 'ik_candidates' field must be of type 'int'"
            assert value >= 0 and value < 4294967296, \
                "The 'ik_candidates' field must be an unsigned integer in [0, 4294967295]"
        self._ik_candidates = value


# Import statements for member types

# already imported above
# import builtins

# already imported above
# import rosidl_parser.definition


class Metaclass_PlanMotion_Feedback(type):
    """Metaclass of message 'PlanMotion_Feedback'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_Feedback')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__action__plan_motion__feedback
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__action__plan_motion__feedback
            cls._CONVERT_TO_PY = module.convert_to_py_msg__action__plan_motion__feedback
            cls._TYPE_SUPPORT = module.type_support_msg__action__plan_motion__feedback
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__action__plan_motion__feedback

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class PlanMotion_Feedback(metaclass=Metaclass_PlanMotion_Feedback):
    """Message class 'PlanMotion_Feedback'."""

    __slots__ = [
        '_stage',
        '_detail',
    ]

    _fields_and_field_types = {
        'stage': 'string',
        'detail': 'string',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.UnboundedString(),  # noqa: E501
        rosidl_parser.definition.UnboundedString(),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.stage = kwargs.get('stage', str())
        self.detail = kwargs.get('detail', str())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.stage != other.stage:
            return False
        if self.detail != other.detail:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def stage(self):
        """Message field 'stage'."""
        return self._stage

    @stage.setter
    def stage(self, value):
        if __debug__:
            assert \
                isinstance(value, str), \
                "The 'stage' field must be of type 'str'"
        self._stage = value

    @builtins.property
    def detail(self):
        """Message field 'detail'."""
        return self._detail

    @detail.setter
    def detail(self, value):
        if __debug__:
            assert \
                isinstance(value, str), \
                "The 'detail' field must be of type 'str'"
        self._detail = value


# Import statements for member types

# already imported above
# import builtins

# already imported above
# import rosidl_parser.definition


class Metaclass_PlanMotion_SendGoal_Request(type):
    """Metaclass of message 'PlanMotion_SendGoal_Request'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_SendGoal_Request')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__action__plan_motion__send_goal__request
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__action__plan_motion__send_goal__request
            cls._CONVERT_TO_PY = module.convert_to_py_msg__action__plan_motion__send_goal__request
            cls._TYPE_SUPPORT = module.type_support_msg__action__plan_motion__send_goal__request
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__action__plan_motion__send_goal__request

            from arm_planning.action import PlanMotion
            if PlanMotion.Goal.__class__._TYPE_SUPPORT is None:
                PlanMotion.Goal.__class__.__import_type_support__()

            from unique_identifier_msgs.msg import UUID
            if UUID.__class__._TYPE_SUPPORT is None:
                UUID.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class PlanMotion_SendGoal_Request(metaclass=Metaclass_PlanMotion_SendGoal_Request):
    """Message class 'PlanMotion_SendGoal_Request'."""

    __slots__ = [
        '_goal_id',
        '_goal',
    ]

    _fields_and_field_types = {
        'goal_id': 'unique_identifier_msgs/UUID',
        'goal': 'arm_planning/PlanMotion_Goal',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.NamespacedType(['unique_identifier_msgs', 'msg'], 'UUID'),  # noqa: E501
        rosidl_parser.definition.NamespacedType(['arm_planning', 'action'], 'PlanMotion_Goal'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        from unique_identifier_msgs.msg import UUID
        self.goal_id = kwargs.get('goal_id', UUID())
        from arm_planning.action._plan_motion import PlanMotion_Goal
        self.goal = kwargs.get('goal', PlanMotion_Goal())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.goal_id != other.goal_id:
            return False
        if self.goal != other.goal:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def goal_id(self):
        """Message field 'goal_id'."""
        return self._goal_id

    @goal_id.setter
    def goal_id(self, value):
        if __debug__:
            from unique_identifier_msgs.msg import UUID
            assert \
                isinstance(value, UUID), \
                "The 'goal_id' field must be a sub message of type 'UUID'"
        self._goal_id = value

    @builtins.property
    def goal(self):
        """Message field 'goal'."""
        return self._goal

    @goal.setter
    def goal(self, value):
        if __debug__:
            from arm_planning.action._plan_motion import PlanMotion_Goal
            assert \
                isinstance(value, PlanMotion_Goal), \
                "The 'goal' field must be a sub message of type 'PlanMotion_Goal'"
        self._goal = value


# Import statements for member types

# already imported above
# import builtins

# already imported above
# import rosidl_parser.definition


class Metaclass_PlanMotion_SendGoal_Response(type):
    """Metaclass of message 'PlanMotion_SendGoal_Response'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_SendGoal_Response')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__action__plan_motion__send_goal__response
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__action__plan_motion__send_goal__response
            cls._CONVERT_TO_PY = module.convert_to_py_msg__action__plan_motion__send_goal__response
            cls._TYPE_SUPPORT = module.type_support_msg__action__plan_motion__send_goal__response
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__action__plan_motion__send_goal__response

            from builtin_interfaces.msg import Time
            if Time.__class__._TYPE_SUPPORT is None:
                Time.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class PlanMotion_SendGoal_Response(metaclass=Metaclass_PlanMotion_SendGoal_Response):
    """Message class 'PlanMotion_SendGoal_Response'."""

    __slots__ = [
        '_accepted',
        '_stamp',
    ]

    _fields_and_field_types = {
        'accepted': 'boolean',
        'stamp': 'builtin_interfaces/Time',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.BasicType('boolean'),  # noqa: E501
        rosidl_parser.definition.NamespacedType(['builtin_interfaces', 'msg'], 'Time'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.accepted = kwargs.get('accepted', bool())
        from builtin_interfaces.msg import Time
        self.stamp = kwargs.get('stamp', Time())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.accepted != other.accepted:
            return False
        if self.stamp != other.stamp:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def accepted(self):
        """Message field 'accepted'."""
        return self._accepted

    @accepted.setter
    def accepted(self, value):
        if __debug__:
            assert \
                isinstance(value, bool), \
                "The 'accepted' field must be of type 'bool'"
        self._accepted = value

    @builtins.property
    def stamp(self):
        """Message field 'stamp'."""
        return self._stamp

    @stamp.setter
    def stamp(self, value):
        if __debug__:
            from builtin_interfaces.msg import Time
            assert \
                isinstance(value, Time), \
                "The 'stamp' field must be a sub message of type 'Time'"
        self._stamp = value


class Metaclass_PlanMotion_SendGoal(type):
    """Metaclass of service 'PlanMotion_SendGoal'."""

    _TYPE_SUPPORT = None

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_SendGoal')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._TYPE_SUPPORT = module.type_support_srv__action__plan_motion__send_goal

            from arm_planning.action import _plan_motion
            if _plan_motion.Metaclass_PlanMotion_SendGoal_Request._TYPE_SUPPORT is None:
                _plan_motion.Metaclass_PlanMotion_SendGoal_Request.__import_type_support__()
            if _plan_motion.Metaclass_PlanMotion_SendGoal_Response._TYPE_SUPPORT is None:
                _plan_motion.Metaclass_PlanMotion_SendGoal_Response.__import_type_support__()


class PlanMotion_SendGoal(metaclass=Metaclass_PlanMotion_SendGoal):
    from arm_planning.action._plan_motion import PlanMotion_SendGoal_Request as Request
    from arm_planning.action._plan_motion import PlanMotion_SendGoal_Response as Response

    def __init__(self):
        raise NotImplementedError('Service classes can not be instantiated')


# Import statements for member types

# already imported above
# import builtins

# already imported above
# import rosidl_parser.definition


class Metaclass_PlanMotion_GetResult_Request(type):
    """Metaclass of message 'PlanMotion_GetResult_Request'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_GetResult_Request')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__action__plan_motion__get_result__request
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__action__plan_motion__get_result__request
            cls._CONVERT_TO_PY = module.convert_to_py_msg__action__plan_motion__get_result__request
            cls._TYPE_SUPPORT = module.type_support_msg__action__plan_motion__get_result__request
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__action__plan_motion__get_result__request

            from unique_identifier_msgs.msg import UUID
            if UUID.__class__._TYPE_SUPPORT is None:
                UUID.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class PlanMotion_GetResult_Request(metaclass=Metaclass_PlanMotion_GetResult_Request):
    """Message class 'PlanMotion_GetResult_Request'."""

    __slots__ = [
        '_goal_id',
    ]

    _fields_and_field_types = {
        'goal_id': 'unique_identifier_msgs/UUID',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.NamespacedType(['unique_identifier_msgs', 'msg'], 'UUID'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        from unique_identifier_msgs.msg import UUID
        self.goal_id = kwargs.get('goal_id', UUID())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.goal_id != other.goal_id:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def goal_id(self):
        """Message field 'goal_id'."""
        return self._goal_id

    @goal_id.setter
    def goal_id(self, value):
        if __debug__:
            from unique_identifier_msgs.msg import UUID
            assert \
                isinstance(value, UUID), \
                "The 'goal_id' field must be a sub message of type 'UUID'"
        self._goal_id = value


# Import statements for member types

# already imported above
# import builtins

# already imported above
# import rosidl_parser.definition


class Metaclass_PlanMotion_GetResult_Response(type):
    """Metaclass of message 'PlanMotion_GetResult_Response'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_GetResult_Response')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__action__plan_motion__get_result__response
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__action__plan_motion__get_result__response
            cls._CONVERT_TO_PY = module.convert_to_py_msg__action__plan_motion__get_result__response
            cls._TYPE_SUPPORT = module.type_support_msg__action__plan_motion__get_result__response
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__action__plan_motion__get_result__response

            from arm_planning.action import PlanMotion
            if PlanMotion.Result.__class__._TYPE_SUPPORT is None:
                PlanMotion.Result.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class PlanMotion_GetResult_Response(metaclass=Metaclass_PlanMotion_GetResult_Response):
    """Message class 'PlanMotion_GetResult_Response'."""

    __slots__ = [
        '_status',
        '_result',
    ]

    _fields_and_field_types = {
        'status': 'int8',
        'result': 'arm_planning/PlanMotion_Result',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.BasicType('int8'),  # noqa: E501
        rosidl_parser.definition.NamespacedType(['arm_planning', 'action'], 'PlanMotion_Result'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.status = kwargs.get('status', int())
        from arm_planning.action._plan_motion import PlanMotion_Result
        self.result = kwargs.get('result', PlanMotion_Result())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.status != other.status:
            return False
        if self.result != other.result:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def status(self):
        """Message field 'status'."""
        return self._status

    @status.setter
    def status(self, value):
        if __debug__:
            assert \
                isinstance(value, int), \
                "The 'status' field must be of type 'int'"
            assert value >= -128 and value < 128, \
                "The 'status' field must be an integer in [-128, 127]"
        self._status = value

    @builtins.property
    def result(self):
        """Message field 'result'."""
        return self._result

    @result.setter
    def result(self, value):
        if __debug__:
            from arm_planning.action._plan_motion import PlanMotion_Result
            assert \
                isinstance(value, PlanMotion_Result), \
                "The 'result' field must be a sub message of type 'PlanMotion_Result'"
        self._result = value


class Metaclass_PlanMotion_GetResult(type):
    """Metaclass of service 'PlanMotion_GetResult'."""

    _TYPE_SUPPORT = None

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_GetResult')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._TYPE_SUPPORT = module.type_support_srv__action__plan_motion__get_result

            from arm_planning.action import _plan_motion
            if _plan_motion.Metaclass_PlanMotion_GetResult_Request._TYPE_SUPPORT is None:
                _plan_motion.Metaclass_PlanMotion_GetResult_Request.__import_type_support__()
            if _plan_motion.Metaclass_PlanMotion_GetResult_Response._TYPE_SUPPORT is None:
                _plan_motion.Metaclass_PlanMotion_GetResult_Response.__import_type_support__()


class PlanMotion_GetResult(metaclass=Metaclass_PlanMotion_GetResult):
    from arm_planning.action._plan_motion import PlanMotion_GetResult_Request as Request
    from arm_planning.action._plan_motion import PlanMotion_GetResult_Response as Response

    def __init__(self):
        raise NotImplementedError('Service classes can not be instantiated')


# Import statements for member types

# already imported above
# import builtins

# already imported above
# import rosidl_parser.definition


class Metaclass_PlanMotion_FeedbackMessage(type):
    """Metaclass of message 'PlanMotion_FeedbackMessage'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion_FeedbackMessage')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__action__plan_motion__feedback_message
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__action__plan_motion__feedback_message
            cls._CONVERT_TO_PY = module.convert_to_py_msg__action__plan_motion__feedback_message
            cls._TYPE_SUPPORT = module.type_support_msg__action__plan_motion__feedback_message
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__action__plan_motion__feedback_message

            from arm_planning.action import PlanMotion
            if PlanMotion.Feedback.__class__._TYPE_SUPPORT is None:
                PlanMotion.Feedback.__class__.__import_type_support__()

            from unique_identifier_msgs.msg import UUID
            if UUID.__class__._TYPE_SUPPORT is None:
                UUID.__class__.__import_type_support__()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
        }


class PlanMotion_FeedbackMessage(metaclass=Metaclass_PlanMotion_FeedbackMessage):
    """Message class 'PlanMotion_FeedbackMessage'."""

    __slots__ = [
        '_goal_id',
        '_feedback',
    ]

    _fields_and_field_types = {
        'goal_id': 'unique_identifier_msgs/UUID',
        'feedback': 'arm_planning/PlanMotion_Feedback',
    }

    SLOT_TYPES = (
        rosidl_parser.definition.NamespacedType(['unique_identifier_msgs', 'msg'], 'UUID'),  # noqa: E501
        rosidl_parser.definition.NamespacedType(['arm_planning', 'action'], 'PlanMotion_Feedback'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
            'Invalid arguments passed to constructor: %s' % \
            ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        from unique_identifier_msgs.msg import UUID
        self.goal_id = kwargs.get('goal_id', UUID())
        from arm_planning.action._plan_motion import PlanMotion_Feedback
        self.feedback = kwargs.get('feedback', PlanMotion_Feedback())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.__slots__, self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s[1:] + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.goal_id != other.goal_id:
            return False
        if self.feedback != other.feedback:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def goal_id(self):
        """Message field 'goal_id'."""
        return self._goal_id

    @goal_id.setter
    def goal_id(self, value):
        if __debug__:
            from unique_identifier_msgs.msg import UUID
            assert \
                isinstance(value, UUID), \
                "The 'goal_id' field must be a sub message of type 'UUID'"
        self._goal_id = value

    @builtins.property
    def feedback(self):
        """Message field 'feedback'."""
        return self._feedback

    @feedback.setter
    def feedback(self, value):
        if __debug__:
            from arm_planning.action._plan_motion import PlanMotion_Feedback
            assert \
                isinstance(value, PlanMotion_Feedback), \
                "The 'feedback' field must be a sub message of type 'PlanMotion_Feedback'"
        self._feedback = value


class Metaclass_PlanMotion(type):
    """Metaclass of action 'PlanMotion'."""

    _TYPE_SUPPORT = None

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('arm_planning')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'arm_planning.action.PlanMotion')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._TYPE_SUPPORT = module.type_support_action__action__plan_motion

            from action_msgs.msg import _goal_status_array
            if _goal_status_array.Metaclass_GoalStatusArray._TYPE_SUPPORT is None:
                _goal_status_array.Metaclass_GoalStatusArray.__import_type_support__()
            from action_msgs.srv import _cancel_goal
            if _cancel_goal.Metaclass_CancelGoal._TYPE_SUPPORT is None:
                _cancel_goal.Metaclass_CancelGoal.__import_type_support__()

            from arm_planning.action import _plan_motion
            if _plan_motion.Metaclass_PlanMotion_SendGoal._TYPE_SUPPORT is None:
                _plan_motion.Metaclass_PlanMotion_SendGoal.__import_type_support__()
            if _plan_motion.Metaclass_PlanMotion_GetResult._TYPE_SUPPORT is None:
                _plan_motion.Metaclass_PlanMotion_GetResult.__import_type_support__()
            if _plan_motion.Metaclass_PlanMotion_FeedbackMessage._TYPE_SUPPORT is None:
                _plan_motion.Metaclass_PlanMotion_FeedbackMessage.__import_type_support__()


class PlanMotion(metaclass=Metaclass_PlanMotion):

    # The goal message defined in the action definition.
    from arm_planning.action._plan_motion import PlanMotion_Goal as Goal
    # The result message defined in the action definition.
    from arm_planning.action._plan_motion import PlanMotion_Result as Result
    # The feedback message defined in the action definition.
    from arm_planning.action._plan_motion import PlanMotion_Feedback as Feedback

    class Impl:

        # The send_goal service using a wrapped version of the goal message as a request.
        from arm_planning.action._plan_motion import PlanMotion_SendGoal as SendGoalService
        # The get_result service using a wrapped version of the result message as a response.
        from arm_planning.action._plan_motion import PlanMotion_GetResult as GetResultService
        # The feedback message with generic fields which wraps the feedback message.
        from arm_planning.action._plan_motion import PlanMotion_FeedbackMessage as FeedbackMessage

        # The generic service to cancel a goal.
        from action_msgs.srv._cancel_goal import CancelGoal as CancelGoalService
        # The generic message for get the status of a goal.
        from action_msgs.msg._goal_status_array import GoalStatusArray as GoalStatusMessage

    def __init__(self):
        raise NotImplementedError('Action classes can not be instantiated')
