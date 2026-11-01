module.exports = [
  {
    type: 'heading',
    defaultValue: 'Infill Clock'
  },
  {
    type: 'text',
    defaultValue: 'Customize time, date, colors, and infill behavior.'
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Time'
      },
      {
        type: 'select',
        messageKey: 'TimeFormat',
        defaultValue: '0',
        label: 'Hour format',
        options: [
          { label: 'System default', value: '0' },
          { label: '12-hour', value: '1' },
          { label: '24-hour', value: '2' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'LeadingZero',
        label: 'Leading zero on hours',
        defaultValue: true
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Date'
      },
      {
        type: 'toggle',
        messageKey: 'ShowDate',
        label: 'Show date',
        defaultValue: false
      },
      {
        type: 'select',
        messageKey: 'DateFormat',
        defaultValue: '0',
        label: 'Date format',
        options: [
          { label: 'Sun 1 Nov', value: '0' },
          { label: 'Nov 1', value: '1' },
          { label: '11/1', value: '2' },
          { label: '1/11', value: '3' }
        ]
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Display'
      },
      {
        type: 'toggle',
        messageKey: 'ShowCaption',
        label: 'Show pattern caption',
        defaultValue: true
      },
      {
        type: 'select',
        messageKey: 'PatternMode',
        defaultValue: '0',
        label: 'Infill pattern',
        options: [
          { label: 'Auto by hour', value: '0' },
          { label: 'Lines', value: '1' },
          { label: 'Honeycomb', value: '2' },
          { label: 'Gyroid', value: '3' },
          { label: 'Triangles', value: '4' }
        ]
      }
    ]
  },

  {
    type: 'section',
    capabilities: ['COLOR'],
    items: [
      {
        type: 'heading',
        defaultValue: 'Colors'
      },
      {
        type: 'color',
        messageKey: 'BackgroundColor',
        defaultValue: '0x000000',
        label: 'Background',
        sunlight: false
      },
      {
        type: 'color',
        messageKey: 'WallColor',
        defaultValue: '0xFFFFFF',
        label: 'Digit walls',
        sunlight: false
      },
      {
        type: 'color',
        messageKey: 'CaptionColor',
        defaultValue: '0xAAAAAA',
        label: 'Caption / date',
        sunlight: false
      },
      {
        type: 'select',
        messageKey: 'FillMode',
        defaultValue: '0',
        label: 'Infill color',
        options: [
          { label: 'Auto by hour', value: '0' },
          { label: 'Custom', value: '1' }
        ]
      },
      {
        type: 'color',
        messageKey: 'FillColor',
        defaultValue: '0xAA00FF',
        label: 'Custom infill color',
        sunlight: false
      }
    ]
  },

  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
